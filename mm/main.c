/*****************************************************************************
 *****************************************************************************
 * KusaOS/mm/main.c
 * Last Modification: by llm,2021/4/17
 * Function: 内存管理模块的核心内容
 
 *****************************************************************************
 *****************************************************************************/

#include "type.h"
#include "config.h"
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

PUBLIC void do_fork_test();

PRIVATE void init_mm();



/*****************************************************************************
 *****************************************************************************
 * task_mm()
 * MM的主消息循环
 * 功能：对从各模块收到的消息进行对应的处理，并发送回复给消息源
 *****************************************************************************
 *****************************************************************************/

PUBLIC void task_mm()
{
	init_mm();

	while (1) {
		send_recv(RECEIVE, ANY, &mm_msg);	/* 获取从其它模块来的MESSAGE */
		int src = mm_msg.source;
		int reply = 1;

		int msgtype = mm_msg.type;
		int tmp=0;
		switch (msgtype) {
		case FORK:
			mm_msg.RETVAL = do_fork(5);
			break;
		case FORK_PRI:
			mm_msg.RETVAL = do_fork(mm_msg.u.m1.m1i1);
			break;
		case EXIT:
			do_exit(mm_msg.STATUS);
			//print_proc_state();
			//if(src==12)print_proc_state();
			reply = 0;
			break;
		case EXEC:
			mm_msg.RETVAL = do_exec();			
			break;
		case WAIT:
			do_wait();
			reply = 0;
			break;
		case PSTAT:
			print_proc_state();
			break;
		case KILLP:
			mm_msg.RETVAL= do_kill();
			break;
		case CSCHED:
			mm_msg.RETVAL=try_change_schedule_policy();
			break;
		case CPROCPRI:
			mm_msg.RETVAL=try_change_proc_priority();
			break;
		case MSTAT:
			tmp=mm_msg.AVAILABLE;
			mm_msg.AVAILABLE=get_proc_memory(tmp);
			break;
		case TOTAL:
			mm_msg.AVAILABLE=memory_size / (1024 * 1024);
			break;
		case BUDDY:
			mm_msg.RETVAL = do_buddy();
			break;
		case MONITOR:
			mm_msg.RETVAL = do_monitor();
			break;
		default:
			dump_msg("MM::unknown msg", &mm_msg);
			assert(0);
			break;
		}
		
		if (reply) {
			mm_msg.type = SYSCALL_RET;
			send_recv(SEND, src, &mm_msg);	 /* 向MESSAGE的来源发送reply */	
			
		}
	}
}


/*****************************************************************************
 *****************************************************************************
 * init_mm()
 * 功能：初始化内存MM模块
 *****************************************************************************
 *****************************************************************************/

PRIVATE void init_mm()
{
	struct boot_params bp;
	get_boot_params(&bp);

	memory_size = bp.mem_size;

	/* 打印内存大小 */
	printl("{MM} memsize:%dMB\n", memory_size / (1024 * 1024));
}

/*****************************************************************************
 *****************************************************************************
 * alloc_mem()
 * 功能:为进程分配内存
 * 参数pid:需要进行内存分配的进程标识
 * 参数memsize:进程所需内存的大小
 * 返回值:为该进程分配内存的基地址
 *****************************************************************************
 *****************************************************************************/

PUBLIC int alloc_mem(int pid, int memsize)
{
	if(pid<0)
	{
		return alloc(-pid,memsize);
	}
	assert(pid >= (NR_TASKS + NR_NATIVE_PROCS));
	if (memsize > PROC_IMAGE_SIZE_DEFAULT) {
		panic("unsupported memory request: %d. "
		      "(should be less than %d)",
		      memsize,
		      PROC_IMAGE_SIZE_DEFAULT);
	}

	int base = PROCS_BASE +
		(pid - (NR_TASKS + NR_NATIVE_PROCS)) * PROC_IMAGE_SIZE_DEFAULT;

	if (base + memsize >= memory_size)
		panic("memory allocation failed. pid:%d", pid);

	return base;
}

/*****************************************************************************
 *****************************************************************************
 * free_mem()
 * 功能:释放内存块
 * 因为每个内存块只对应一个pid，所以事实上该函数里什么也不用做
 * 参数pid:释放内存的进程pid
 * 返回值:0
 *****************************************************************************
 *****************************************************************************/

PUBLIC int free_mem(int pid)
{
	return 0;
}


/*****************************************************************************
 *****************************************************************************
 * print_proc_state()
 * 功能:打印进程状态
 *  
 * 返回值:0
 *****************************************************************************
 *****************************************************************************/
PUBLIC int print_proc_state()
{
	int i;
	printl("=============================================================================\n");
    printl("   type    |    pid    |    name   | spriority |  state  |  parent\n");
	for (i = 0; i < NR_TASKS + NR_PROCS; i++) {
		
		if (i < NR_TASKS ) { /* TASK */
			printl("   TASK    ");
			printl("%12d%12s%12d%12d",
			i, 
			proc_table[i].name, 		
			proc_table[i].priority,
			proc_table[i].p_flags
			);
			if(proc_table[i].p_parent<NR_TASKS+NR_PROCS)
				printl("%12d\n",proc_table[i].p_parent);
			else
				printl("           -\n");
		}
		else{ /* USER_PROC */
			if(proc_table[i].p_flags==FREE_SLOT)
				continue;
			printl(" USER_PROC ");
			printl("%12d%12s%12d%12d",
			i, 
			proc_table[i].name, 		
			proc_table[i].priority,
			proc_table[i].p_flags
			);
			if(proc_table[i].p_parent<NR_TASKS+NR_PROCS)
				printl("%12d\n",proc_table[i].p_parent);
			else
				printl("           -\n");
		}
	}	
	return 0;
}
/*****************************************************************************
 *****************************************************************************
 * do_kill
 * 功能:杀死进程
 *  
 * 返回值: 0 -- kill成功  1 -- kill失败
 *****************************************************************************
 *****************************************************************************/
PUBLIC int do_kill(){
	int pid =mm_msg.PID;
	if((pid<NR_TASKS+NR_NATIVE_PROCS+3)|/* 无法杀死系统进程和初始化进程 */
		(pid<0|pid>=NR_TASKS+NR_PROCS)|/* pid 越界*/
		proc_table[pid].p_flags==FREE_SLOT)/* 对应进程非运行状态 */ 
		return 1;
	
	do_exit_by_mm(pid);
		return 0;
}
/*****************************************************************************
 *****************************************************************************
 * try_change_schedule_policy
 * 功能:改变调度策略
 *  
 * 返回值: 当前调度策略
 *****************************************************************************
 *****************************************************************************/
PUBLIC int try_change_schedule_policy(){
	int policy =mm_msg.SCHED_POL;
	if(policy == SCHED_DEFAULT||
		policy == SCHED_FIFO||
		policy == SCHED_PRI||
		policy == SCHED_PRI_DY||
		policy == SCHED_RR){
			schedule_policy = policy;
		}
	return schedule_policy;
}
/*****************************************************************************
 *****************************************************************************
 * try_change_proc_priority
 * 功能:改变进程优先级
 *  
 * 返回值: -1 失败 0 成功
 *****************************************************************************
 *****************************************************************************/
PUBLIC int try_change_proc_priority(){
	int pid = mm_msg.PID;
	if((pid<NR_TASKS)|/* 无法修改系统进程和初始化进程 */
		(pid<0|pid>=NR_TASKS+NR_PROCS)|/* pid 越界*/
		proc_table[pid].p_flags==FREE_SLOT)/* 对应进程非运行状态 */ 
		return -1;	/* change fail */
	int priority = mm_msg.PRI;
	if(priority<=0||priority>=MAX_PRIORITY)
		priority = DEFAULT_USER_PROC_PRIORITY;
	proc_table[pid].priority = mm_msg.PRI;
	return 0;
}

/*****************************************************************************
 *****************************************************************************
 * get_proc_memory
 * 功能:根据pid获取进程的内存基地址
 * 参数:pid进程标识
 * 返回值:0 
 *****************************************************************************
 *****************************************************************************/
PUBLIC int get_proc_memory(int pid)
{
	int base = PROCS_BASE +
		(pid - (NR_TASKS + NR_NATIVE_PROCS)) * PROC_IMAGE_SIZE_DEFAULT;
	return base;
}


/*****************************************************************************
 *****************************************************************************
 * set_child,set_parent
 * 功能:buddy算法的辅助函数，用于设置标记
 *****************************************************************************
 *****************************************************************************/

int mem_buf[BUDDY_SIZE * 2];
PUBLIC void set_child(int i)			//对子结点标记
{
	if (i < BUDDY_SIZE * 2 - 1)
	{
		mem_buf[i] = 1;
		set_child(2*i+1);
		set_child(2*i+2);
	}
}
PUBLIC void set_parent(int i)			//对父节点标记
{
	int p =(i-1)/2;
	while (p >= 0)
	{
		mem_buf[p] = 1;
		if (p == 0) return;
		p = (p-1)/2;
	}
}


/*****************************************************************************
 *****************************************************************************
 * alloc
 * 功能:用于buddy算法的分配程序
 * 
 * 返回值:分配的基地址
 *****************************************************************************
 *****************************************************************************/
PUBLIC int alloc(int seq,int sz)		
{
	//index是对应二叉树的结点编号，addr是内存起始地址
	//cur_sz表示分配内存块的实际大小（包含了碎片部分）
	int index = 0, cur_sz = BUDDY_SIZE, addr = 0, i;
	for (cur_sz = BUDDY_SIZE; cur_sz / 2 >= sz; cur_sz /= 2)
		index = 2*index+1;

	for (i = index; i <= index * 2; i++, addr += cur_sz)
	{
		if (!mem_buf[i])
		{
			set_child(i);
			set_parent(i);
			return addr;
		}
	}
}


/*****************************************************************************
 *****************************************************************************
 * do_buddy
 * 功能:生成随机大小的页数，执行buddy分配算法分配内存
 * 参数:mm_msg
 * 返回值:0 成功
 *****************************************************************************
 *****************************************************************************/
PUBLIC int do_buddy()
{
	srand(get_ticks());
	int i;
	for(i=0;i<BUDDY_SIZE * 2;i++)
		mem_buf[i]=0;
	for(i=1;i<=10;i++)
	{
		int random_number=rand()%29+4;
		int addr=alloc_mem(-i,random_number)*1024+PAGE_START;
		mm_msg.LONG=random_number;
		mm_msg.LONG2=addr;
		mm_msg.type = SYSCALL_RET;
		send_recv(BOTH, mm_msg.source, &mm_msg);	 /* 向MESSAGE的来源发送reply */
	}
	return 0;
}

/*****************************************************************************
 *****************************************************************************
 * do_monitor()
 * 功能:把进程表各pid是否空闲交给监视进程的程序
 *  
 * 返回值:  0 成功
 *****************************************************************************
 *****************************************************************************/

PUBLIC int do_monitor() {
	mm_msg.LONG = 0;
	int i;
	u64 flag = 1;
	for (i = 0; i < NR_TASKS + NR_PROCS; i++) {
		if(proc_table[i].p_flags !=FREE_SLOT)
			mm_msg.LONG |= flag;
		flag = flag << 1;
	}	

	return 0;
}
