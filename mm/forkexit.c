/*****************************************************************************
 *****************************************************************************
 * KusaOS/mm/forkexit.c
 * Last Modification: by llm,2021/4/17
 * Function: 系统调用fork(),wait(),exit()的具体实现
 
 *****************************************************************************
 *****************************************************************************/

#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"


PRIVATE void cleanup(struct proc * proc);

/*****************************************************************************
 *****************************************************************************
 * do_fork()
 * 功能：执行系统调用fork()
 * 返回值：成功fork返回0，否则返回-1
 * 1、分配进程表：从数组proc_table[ ]中寻找一个空项，用于存放子进程的进程表。然后将父进程的进程表原原本本地赋给子进程。
 * 2、分配内存：先获取父进程的内存占用情况，然后使用alloc_mem()函数分配内存
 * 3、将父进程的内存空间完整地复制了一份给新分配的空间。然后更新子进程的LDT
 * 4、通知文件系统，使得父进程和子进程能够共享文件。
 * 5、向子进程发送消息，解除其阻塞，将0作为返回值传递给子进程，以便让他知道自己的身份。父进程则有MM的主消息循环通知。
 
 *****************************************************************************
 *****************************************************************************/

PUBLIC int do_fork(int priority)
{
	/* 在进程表中找到一个空项 */
	struct proc* p = proc_table;
	int i;
	for (i = 0; i < NR_TASKS + NR_PROCS; i++,p++)
		if (p->p_flags == FREE_SLOT)
			break;

	int child_pid = i;
	assert(p == &proc_table[child_pid]);
	assert(child_pid >= NR_TASKS + NR_NATIVE_PROCS);
	if (i == NR_TASKS + NR_PROCS) /* no free slot */
		return -1;
	assert(i < NR_TASKS + NR_PROCS);

	/* 拷贝进程表 */
	int pid = mm_msg.source;
	u16 child_ldt_sel = p->ldt_sel;
	*p = proc_table[pid];
	p->ldt_sel = child_ldt_sel;
	p->p_parent = pid;
	sprintf(p->name, "%s_%d", proc_table[pid].name, child_pid);

	struct descriptor * ppd;

	ppd = &proc_table[pid].ldts[INDEX_LDT_C];
	int caller_T_base  = reassembly(ppd->base_high, 24,
					ppd->base_mid,  16,
					ppd->base_low);
	int caller_T_limit = reassembly(0, 0,
					(ppd->limit_high_attr2 & 0xF), 16,
					ppd->limit_low);
	int caller_T_size  = ((caller_T_limit + 1) *
			      ((ppd->limit_high_attr2 & (DA_LIMIT_4K >> 8)) ?
			       4096 : 1));

	ppd = &proc_table[pid].ldts[INDEX_LDT_RW];
	int caller_D_S_base  = reassembly(ppd->base_high, 24,
					  ppd->base_mid,  16,
					  ppd->base_low);
	int caller_D_S_limit = reassembly((ppd->limit_high_attr2 & 0xF), 16,
					  0, 0,
					  ppd->limit_low);
	int caller_D_S_size  = ((caller_T_limit + 1) *
				((ppd->limit_high_attr2 & (DA_LIMIT_4K >> 8)) ?
				 4096 : 1));

	//设置进程优先级
	p->priority = priority;
	
	assert((caller_T_base  == caller_D_S_base ) &&
	       (caller_T_limit == caller_D_S_limit) &&
	       (caller_T_size  == caller_D_S_size ));

	/* 给子进程分配内存 */
	int child_base = alloc_mem(child_pid, caller_T_size);

	/* 将父进程的内存空间完整地复制了一份给新分配的空间 */
	phys_copy((void*)child_base, (void*)caller_T_base, caller_T_size);

	/* 更新子进程的LDT */
	init_desc(&p->ldts[INDEX_LDT_C],
		  child_base,
		  (PROC_IMAGE_SIZE_DEFAULT - 1) >> LIMIT_4K_SHIFT,
		  DA_LIMIT_4K | DA_32 | DA_C | PRIVILEGE_USER << 5);
	init_desc(&p->ldts[INDEX_LDT_RW],
		  child_base,
		  (PROC_IMAGE_SIZE_DEFAULT - 1) >> LIMIT_4K_SHIFT,
		  DA_LIMIT_4K | DA_32 | DA_DRW | PRIVILEGE_USER << 5);

	/* 通知文件系统，使得父进程和子进程能够共享文件 */
	MESSAGE msg2fs;
	msg2fs.type = FORK;
	msg2fs.PID = child_pid;
	send_recv(BOTH, TASK_FS, &msg2fs);

	/* 向父进程返回子进程PID */
	mm_msg.PID = child_pid;

	/* 子进程诞生 */
	MESSAGE m;
	m.type = SYSCALL_RET;
	m.RETVAL = 0;
	m.PID = 0;
	send_recv(SEND, child_pid, &m);

	return 0;
}
/*****************************************************************************
 *****************************************************************************
 * do_exit_by_mm(int pid)
 * 功能：由mm为pid执行系统调用exit()
 * 参数status：父进程的existing status位
 *
 *****************************************************************************
 *****************************************************************************/
PUBLIC void do_exit_by_mm(int pid)
{
	int i;
	//int pid = mm_msg.source; /* 调用进程的PID */
	int parent_pid = proc_table[pid].p_parent;
	struct proc * p = &proc_table[pid];

	/* 通知文件系统 */
	MESSAGE msg2fs;
	msg2fs.type = EXIT;
	msg2fs.PID = pid;
	send_recv(BOTH, TASK_FS, &msg2fs);

	free_mem(pid);

	p->exit_status = pid;

	if (proc_table[parent_pid].p_flags & WAITING) { /*父进程正在waiting */
		proc_table[parent_pid].p_flags &= ~WAITING;
		cleanup(&proc_table[pid]);
	}
	else { 
		proc_table[pid].p_flags |= HANGING;
	}

	/* 如果他有子进程，则将INIT进程作为子进程的新父进程 */
	for (i = 0; i < NR_TASKS + NR_PROCS; i++) {
		if (proc_table[i].p_parent == pid) { /* 是子进程 */
			proc_table[i].p_parent = INIT;
			if ((proc_table[INIT].p_flags & WAITING) &&
			    (proc_table[i].p_flags & HANGING)) {
				proc_table[INIT].p_flags &= ~WAITING;
				cleanup(&proc_table[i]);
			}
		}
	}
}
/*****************************************************************************
 *****************************************************************************
 * do_exit()
 * 功能：执行系统调用exit()
 * 参数status：父进程的existing status位
 *
 * 假设进程P有子进程A。而A调用exit( )，那么MM将会：
 * 1. 告诉FS：A退出，请做相应处理
 * 2. 释放A占用的内存
 * 3. 判断P是否正在WAITING
 * 	如果是：
 * 		清除P的WAITING位
 * 		向P发送消息以解除阻塞（到此P的wait( )函数结束）
 * 		释放A的进程表项（到此A的exit( )函数结束）
 * 	如果否：
 * 		设置A的HANGING位
 * 4. 遍历proc_table[ ]，如果发现A有子进程B，那么
 * 		将Init进程设置为B的父进程（换言之，将B过继给Init）
 * 		判断是否满足Init正在WAITING且B正在HANGING
 * 		如果是：
 * 			清除Init的WAITING位
 * 			向Init发送消息以解除阻塞（到此Init的wait( )函数结束）
 * 			释放B的进程表项（到此B的exit( )函数结束）
 * 		如果否：
 * 			如果Init正在WAITING但B并没有HANGING，将来B调用exit( )时执行上过程
 * 			如果B正在HANGING但Init并没有WAITING，将来Init调用wait( )时执行上过程
 * 
 *****************************************************************************
 *****************************************************************************/
PUBLIC void do_exit(int status)
{
	int i;
	int pid = mm_msg.source; /* 调用进程的PID */
	int parent_pid = proc_table[pid].p_parent;
	struct proc * p = &proc_table[pid];

	/* 通知文件系统 */
	MESSAGE msg2fs;
	msg2fs.type = EXIT;
	msg2fs.PID = pid;
	send_recv(BOTH, TASK_FS, &msg2fs);

	free_mem(pid);

	p->exit_status = status;

	if (proc_table[parent_pid].p_flags & WAITING) { /*父进程正在waiting */
		proc_table[parent_pid].p_flags &= ~WAITING;
		cleanup(&proc_table[pid]);
	}
	else { 
		proc_table[pid].p_flags |= HANGING;
	}

	/* 如果他有子进程，则将INIT进程作为子进程的新父进程 */
	for (i = 0; i < NR_TASKS + NR_PROCS; i++) {
		if (proc_table[i].p_parent == pid) { /* 是子进程 */
			proc_table[i].p_parent = INIT;
			if ((proc_table[INIT].p_flags & WAITING) &&
			    (proc_table[i].p_flags & HANGING)) {
				proc_table[INIT].p_flags &= ~WAITING;
				cleanup(&proc_table[i]);
			}
		}
	}
}

/*****************************************************************************
 *****************************************************************************
 * cleanup()
 * 功能：向父进程发送消息以解除其阻塞，同时释放子进程表项
 
 *****************************************************************************
 *****************************************************************************/
PRIVATE void cleanup(struct proc * proc)
{
	MESSAGE msg2parent;
	msg2parent.type = SYSCALL_RET;
	msg2parent.PID = proc2pid(proc);
	msg2parent.STATUS = proc->exit_status;
	send_recv(SEND, proc->p_parent, &msg2parent);

	proc->p_flags = FREE_SLOT;
}

/*****************************************************************************
 *****************************************************************************
 * do_wait()
 * 功能：执行系统调用wait()
 *
 * 如果P调用wait( )，那么MM将会：
 * 1. 遍历proc_tabel[ ]，如果发现A是P的子进程，并且它正在HANGING，那么
 * 		向P发送消息以解除阻塞（到此P的wait( )函数结束）
 * 		释放A的进程表项（到此A的exit( )函数结束）
 * 2. 如果P的子进程没有一个在HANGING，则
 * 		设P的WAITING位
 * 3. 如果P压根儿没有子进程，则
 *  		向P发送消息，消息携带一个表示出错的返回值（到此P的wait( )函数结束）
 * 
 * 注：
 * HANGING：进程除了进程表项（PCB）以外资源已经全部释放
 * WAITING：进程至少拥有一个子进程，并且在等待子进程的退出
 *
 *****************************************************************************
 *****************************************************************************/
PUBLIC void do_wait()
{
	int pid = mm_msg.source;

	int i;
	int children = 0;
	struct proc* p_proc = proc_table;
	for (i = 0; i < NR_TASKS + NR_PROCS; i++,p_proc++) {
		if (p_proc->p_parent == pid) {
			children++;
			if (p_proc->p_flags & HANGING) {
				cleanup(p_proc);
				return;
			}
		}
	}

	if (children) {
		/* 子进程没有一个在HANGING，则设P的WAITING位 */
		proc_table[pid].p_flags |= WAITING;
	}
	else {
		/* 没有子进程 */
		MESSAGE msg;
		msg.type = SYSCALL_RET;
		msg.PID = NO_TASK;
		send_recv(SEND, pid, &msg);
	}
}
