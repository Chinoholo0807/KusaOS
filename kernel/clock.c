/*************************************************************************//**
 *****************************************************************************
 * @file   clock.c
 * @brief  时钟相关
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
#include "proto.h"


/*****************************************************************************
 *                                clock_handler
 *****************************************************************************/
/**
 * <Ring 0> This routine handles the clock interrupt generated by 8253/8254
 *          programmable interval timer.
 * 时钟中断处理函数，当发生时钟中断时，在调用save后会调用该函数
 * @param irq The IRQ nr, unused here.
 *****************************************************************************/
PUBLIC void clock_handler(int irq)
{
	if (++ticks >= MAX_TICKS)
		ticks = 0;
	
	if(schedule_policy !=4 ){
> 		if (key_pressed)
> 			inform_int(TASK_TTY);
> 		if (k_reenter != 0) {
> 			return;
> 		}
> 		schedule();
> 		return;
> 	}
	
	if (p_proc_ready->ticks)
		p_proc_ready->ticks--;

	if (key_pressed)
		inform_int(TASK_TTY);

	if (k_reenter != 0) {
		return;
	}

	if (p_proc_ready->ticks > 0) {
		return;
	}

	schedule();/* 使用进程调度函数schedule() */

}

/*****************************************************************************
 *                                milli_delay
 *****************************************************************************/
/**
 * <Ring 1~3> Delay for a specified amount of time.
 * 延迟函数，默认系统中时钟中断发生周期为10ms，例如milli_sec=100，则会等待10次时钟中断
 * @param milli_sec How many milliseconds to delay.
 *****************************************************************************/
PUBLIC void milli_delay(int milli_sec)
{
        int t = get_ticks();

        while(((get_ticks() - t) * 1000 / HZ) < milli_sec) {}
}

/*****************************************************************************
 *                                init_clock
 *****************************************************************************/
/**
 * <Ring 0> Initialize 8253/8254 PIT (Programmable Interval Timer).
 * 时钟初始化函数，将clock_hnadler与时钟中断号绑定，同时修改时钟中断发生周期为10ms
 *****************************************************************************/
PUBLIC void init_clock()
{
        /* 初始化 8253 PIT */
        out_byte(TIMER_MODE, RATE_GENERATOR);
        out_byte(TIMER0, (u8) (TIMER_FREQ/HZ) );
        out_byte(TIMER0, (u8) ((TIMER_FREQ/HZ) >> 8));

        put_irq_handler(CLOCK_IRQ, clock_handler);    /* 设定时钟中断处理程序 */
        enable_irq(CLOCK_IRQ);                        /* 让8259A可以接收时钟中断 */
}


