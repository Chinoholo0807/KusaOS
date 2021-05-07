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
			mm_msg.RETVAL = do_fork();
			break;
		case EXIT:
			do_exit(mm_msg.STATUS);
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
		case MSTAT:
			tmp=mm_msg.u.m1.m1i1;
			//printf("main.c %d\n",tmp);
			print_proc_memory(tmp);
			break;
		case TOTAL:
			printl("{MM} memsize:%dMB\n", memory_size / (1024 * 1024));
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
    printl("   type    |    pid    |    name   | spriority |  state\n");
	for (i = 0; i < NR_TASKS + NR_PROCS; i++) {
		
		if (i < NR_TASKS ) { /* TASK */
			printl("   TASK    ");
			printl("%12d%12s%12d%12d\n", i, proc_table[i].name, 		
										proc_table[i].priority,proc_table[i].p_flags);
		}
		else{ /* USER_PROC */
			if(proc_table[i].p_flags==FREE_SLOT)
				continue;
			printl(" USER_PROC ");
        	printl("%12d%12s%12d%12d\n", i, proc_table[i].name, 		
											proc_table[i].priority,proc_table[i].p_flags);
		}
	}	
	return 0;
}

PUBLIC int print_proc_memory(int pid)
{
	int base = PROCS_BASE +
		(pid - (NR_TASKS + NR_NATIVE_PROCS)) * PROC_IMAGE_SIZE_DEFAULT;
	printl("pid: %d   mem_base: %d\n",pid,base);
	return 0;
}
