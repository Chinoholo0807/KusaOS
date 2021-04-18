/*************************************************************************//**
 *****************************************************************************
 * @file   kernel/tty.c
 * @brief  终端设备
 *
 * 作为一个设备TTY接受这些消息：
 *   - 打开设备
 *   - 读设备
 *   - 写设备
 *
 * 此外，还从时钟中断处理函数和不是文件系统任务的进程接受两种消息
 *
 *   - 时钟中断处理函数发来的消息：HARD_INT
 *      - 每次时钟中断发生就去从键盘读取数据到缓冲区
 *
 *   - 文件系统任务外的进程发来的消息： TTY_WRITE
 *      - TTY是一个设备 大多数情况下消息都是经由文件系统从进程发送给TTY
 *       处于历史原因, 进程允许直接向TTY发送写消息
 *       因此进程可以直接写TTY
 *
 * @note   请不要对下列的函数名感到疑惑与困扰:
 *           - tty_dev_read() vs tty_do_read()
 *             - tty_dev_read() 从键盘缓冲读取字符
 *             - tty_do_read() 处理DEV_READ消息
 *           - tty_dev_write() vs tty_do_write() vs tty_write()
 *             - tty_dev_write() 向请求的进程返回数据
 *             - tty_do_write() 处理DEV_WRITE消息
 *             - tty_write() 处理TTY_WRITE消息
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


#define TTY_FIRST	(tty_table)
#define TTY_END		(tty_table + NR_CONSOLES)


PRIVATE void	init_tty	(TTY* tty);
PRIVATE void	tty_dev_read	(TTY* tty);
PRIVATE void	tty_dev_write	(TTY* tty);
PRIVATE void	tty_do_read	(TTY* tty, MESSAGE* msg);
PRIVATE void	tty_do_write	(TTY* tty, MESSAGE* msg);
PRIVATE void	put_key		(TTY* tty, u32 key);


/*****************************************************************************
 *                                task_tty
 *****************************************************************************/
/**
 * <Ring 1> TTY任务的主循环
 *****************************************************************************/
PUBLIC void task_tty()
{
	TTY *	tty;
	MESSAGE msg;

	init_keyboard();

	for (tty = TTY_FIRST; tty < TTY_END; tty++)
		init_tty(tty);

	select_console(0);

	while (1) {
		for (tty = TTY_FIRST; tty < TTY_END; tty++) {
			do {
				tty_dev_read(tty);
				tty_dev_write(tty);
			} while (tty->ibuf_cnt);
		}

		send_recv(RECEIVE, ANY, &msg);

		int src = msg.source;
		assert(src != TASK_TTY);

		TTY* ptty = &tty_table[msg.DEVICE];

		switch (msg.type) {
		case DEV_OPEN:
			reset_msg(&msg);
			msg.type = SYSCALL_RET;
			send_recv(SEND, src, &msg);
			break;
		case DEV_READ:
			tty_do_read(ptty, &msg);
			break;
		case DEV_WRITE:
			tty_do_write(ptty, &msg);
			break;
		case HARD_INT:
			/**
			 * 被时钟中断处理函数唤醒 -- 一个按键被按下
			 * @see clock_handler() inform_int()
			 */
			key_pressed = 0;
			continue;
		default:
			dump_msg("TTY::unknown msg", &msg);
			break;
		}
	}
}


/*****************************************************************************
 *                                init_tty
 *****************************************************************************/
/**
 * 在TTY可以被使用之前需要被初始化
 *   -# the input buffer
 *   -# the corresponding console
 * 
 * @param tty  TTY stands for teletype, a cool ancient magic thing.
 *****************************************************************************/
PRIVATE void init_tty(TTY* tty)
{
	tty->ibuf_cnt = 0;
	tty->ibuf_head = tty->ibuf_tail = tty->ibuf;

	init_screen(tty);
}


/*****************************************************************************
 *                                in_process
 *****************************************************************************/
/**
 * keyboard_read()在识别完按键后会使用这个函数
 * 
 * @param tty  The key press is for whom.
 * @param key  The integer key with metadata.
 *****************************************************************************/
PUBLIC void in_process(TTY* tty, u32 key)
{
	if (!(key & FLAG_EXT)) {
		put_key(tty, key);
	}
	else {
		int raw_code = key & MASK_RAW;
		switch(raw_code) {
		case ENTER:
			put_key(tty, '\n');
			break;
		case BACKSPACE:
			put_key(tty, '\b');
			break;
		case UP:
			if ((key & FLAG_SHIFT_L) ||
			    (key & FLAG_SHIFT_R)) {	/* Shift + Up */
				scroll_screen(tty->console, SCR_DN);
			}
			break;
		case DOWN:
			if ((key & FLAG_SHIFT_L) ||
			    (key & FLAG_SHIFT_R)) {	/* Shift + Down */
				scroll_screen(tty->console, SCR_UP);
			}
			break;
		case F1:
		case F2:
		case F3:
		case F4:
		case F5:
		case F6:
		case F7:
		case F8:
		case F9:
		case F10:
		case F11:
		case F12:
			if ((key & FLAG_ALT_L) ||
			    (key & FLAG_ALT_R)) {	/* Alt + F1~F12 */
				select_console(raw_code - F1);
			}
			break;
		default:
			break;
		}
	}
}


/*****************************************************************************
 *                                put_key
 *****************************************************************************/
/**
 * 把识别到的按键放入TTY的in缓冲区
 *
 * @callergraph
 * 
 * @param tty  To which TTY the key is put.
 * @param key  The key. It's an integer whose higher 24 bits are metadata.
 *****************************************************************************/
PRIVATE void put_key(TTY* tty, u32 key)
{
	if (tty->ibuf_cnt < TTY_IN_BYTES) {
		*(tty->ibuf_head) = key;
		tty->ibuf_head++;
		if (tty->ibuf_head == tty->ibuf + TTY_IN_BYTES)
			tty->ibuf_head = tty->ibuf;
		tty->ibuf_cnt++;
	}
}


/*****************************************************************************
 *                                tty_dev_read
 *****************************************************************************/
/**
 * 从键盘缓冲得到字符如果TTY::console是当前的控制台
 *
 * @see keyboard_read()
 * 
 * @param tty  Ptr to TTY.
 *****************************************************************************/
PRIVATE void tty_dev_read(TTY* tty)
{
	if (is_current_console(tty->console))
		keyboard_read(tty);
}


/*****************************************************************************
 *                                tty_dev_write
 *****************************************************************************/
/**
 * 回显刚刚键入的字符
 * 
 * @param tty   Ptr to a TTY struct.
 *****************************************************************************/
PRIVATE void tty_dev_write(TTY* tty)
{
	while (tty->ibuf_cnt) {
		char ch = *(tty->ibuf_tail);
		tty->ibuf_tail++;
		if (tty->ibuf_tail == tty->ibuf + TTY_IN_BYTES)
			tty->ibuf_tail = tty->ibuf;
		tty->ibuf_cnt--;

		if (tty->tty_left_cnt) {
			if (ch >= ' ' && ch <= '~') { /*可打印的 */
				out_char(tty->console, ch);
				void * p = tty->tty_req_buf +
					   tty->tty_trans_cnt;
				phys_copy(p, (void *)va2la(TASK_TTY, &ch), 1);
				tty->tty_trans_cnt++;
				tty->tty_left_cnt--;
			}
			else if (ch == '\b' && tty->tty_trans_cnt) {
				out_char(tty->console, ch);
				tty->tty_trans_cnt--;
				tty->tty_left_cnt++;
			}

			if (ch == '\n' || tty->tty_left_cnt == 0) {
				out_char(tty->console, '\n');
				MESSAGE msg;
				msg.type = RESUME_PROC;
				msg.PROC_NR = tty->tty_procnr;
				msg.CNT = tty->tty_trans_cnt;
				send_recv(SEND, tty->tty_caller, &msg);
				tty->tty_left_cnt = 0;
			}
		}
	}
}


/*****************************************************************************
 *                                tty_do_read
 *****************************************************************************/
/**
 * 当TTY任务接收到DEV_READ消息时被调动
 *
 * @该函数会在设置TTY结构体的一些成员，告诉文件系统去阻塞请求读的进程后立刻返回。
 * 真正处理(tty buffer -> proc buffer)的函数不在这里.
 * 
 * @param tty  From which TTY the caller proc wants to read.
 * @param msg  The MESSAGE just received.
 *****************************************************************************/
PRIVATE void tty_do_read(TTY* tty, MESSAGE* msg)
{
	/* 通知tty: */
	tty->tty_caller   = msg->source;  /* 是谁调用的，一般来说是文件系统 */
	tty->tty_procnr   = msg->PROC_NR; /* 请求的进程号*/
	tty->tty_req_buf  = va2la(tty->tty_procnr,
				  msg->BUF);/*字符输出到哪里 */
	tty->tty_left_cnt = msg->CNT; /*需要多少个字符 */
	tty->tty_trans_cnt= 0; /* 已经传输的字符数量 */

	msg->type = SUSPEND_PROC;
	msg->CNT = tty->tty_left_cnt;
	send_recv(SEND, tty->tty_caller, msg);
}


/*****************************************************************************
 *                                tty_do_write
 *****************************************************************************/
/**
 * 在TTY任务接收到DEV_WRITE消息时被调用
 * 
 * @param tty  To which TTY the calller proc is bound.
 * @param msg  The MESSAGE.
 *****************************************************************************/
PRIVATE void tty_do_write(TTY* tty, MESSAGE* msg)
{
	char buf[TTY_OUT_BUF_LEN];
	char * p = (char*)va2la(msg->PROC_NR, msg->BUF);
	int i = msg->CNT;
	int j;

	while (i) {
		int bytes = min(TTY_OUT_BUF_LEN, i);
		phys_copy(va2la(TASK_TTY, buf), (void*)p, bytes);
		for (j = 0; j < bytes; j++)
			out_char(tty->console, buf[j]);
		i -= bytes;
		p += bytes;
	}

	msg->type = SYSCALL_RET;
	send_recv(SEND, msg->source, msg);
}


/*****************************************************************************
 *                                sys_printx
 *****************************************************************************/
/**
 * 系统调用支持四个参数`printx' 只需要两个
 *
 * @note `printx' accepts only one parameter -- `char* s', the other one --
 * `struct proc * proc' -- is pushed by kernel.asm::sys_call so that the
 * kernel can easily know who invoked the system call.
 *
 * @note s[0] (the first char of param s) is a magic char. if it equals
 * MAG_CH_PANIC, then this syscall was invoked by `panic()', which means
 * something goes really wrong and the system is to be halted; if it equals
 * MAG_CH_ASSERT, then this syscall was invoked by `assert()', which means
 * an assertion failure has occured. @see kernel/main lib/misc.c.
 * 
 * @param _unused1  Ignored.
 * @param _unused2  Ignored.
 * @param s         The string to be printed.
 * @param p_proc    Caller proc.
 * 
 * @return  Zero if success.
 *****************************************************************************/
PUBLIC int sys_printx(int _unused1, int _unused2, char* s, struct proc* p_proc)
{
	const char * p;
	char ch;

	char reenter_err[] = "? k_reenter is incorrect for unknown reason";
	reenter_err[0] = MAG_CH_PANIC;

	/**
	 * @note Code in both Ring 0 and Ring 1~3 may invoke printx().
	 * If this happens in Ring 0, no linear-physical address mapping
	 * is needed.
	 *
	 * @attention The value of `k_reenter' is tricky here. When
	 *   -# printx() is called in Ring 0
	 *      - k_reenter > 0. When code in Ring 0 calls printx(),
	 *        an `interrupt re-enter' will occur (printx() generates
	 *        a software interrupt). Thus `k_reenter' will be increased
	 *        by `kernel.asm::save' and be greater than 0.
	 *   -# printx() is called in Ring 1~3
	 *      - k_reenter == 0.
	 */
	if (k_reenter == 0)  /* printx() called in Ring<1~3> */
		p = va2la(proc2pid(p_proc), s);
	else if (k_reenter > 0) /* printx() called in Ring<0> */
		p = s;
	else	/* this should NOT happen */
		p = reenter_err;

	/**
	 * @note if assertion fails in any TASK, the system will be halted;
	 * if it fails in a USER PROC, it'll return like any normal syscall
	 * does.
	 */
	if ((*p == MAG_CH_PANIC) ||
	    (*p == MAG_CH_ASSERT && p_proc_ready < &proc_table[NR_TASKS])) {
		disable_int();
		char * v = (char*)V_MEM_BASE;
		const char * q = p + 1; /* +1: skip the magic char */

		while (v < (char*)(V_MEM_BASE + V_MEM_SIZE)) {
			*v++ = *q++;
			*v++ = RED_CHAR;
			if (!*q) {
				while (((int)v - V_MEM_BASE) % (SCR_WIDTH * 16)) {
					/* *v++ = ' '; */
					v++;
					*v++ = GRAY_CHAR;
				}
				q = p + 1;
			}
		}

		__asm__ __volatile__("hlt");
	}

	while ((ch = *p++) != 0) {
		if (ch == MAG_CH_PANIC || ch == MAG_CH_ASSERT)
			continue; /* skip the magic char */

		/* TTY * ptty; */
		/* for (ptty = TTY_FIRST; ptty < TTY_END; ptty++) */
		/* 	out_char(ptty->console, ch); /\* output chars to all TTYs *\/ */
		out_char(TTY_FIRST->console, ch);
	}

	//__asm__ __volatile__("nop;jmp 1f;ud2;1: nop");
	//__asm__ __volatile__("nop;cli;1: jmp 1b;ud2;nop");

	return 0;
}

/*****************************************************************************
 *                                dump_tty_buf
 *****************************************************************************/
/**
 * 仅仅是debug用的
 * 
 *****************************************************************************/
PUBLIC void dump_tty_buf()
{
	TTY * tty = &tty_table[1];

	static char sep[] = "--------------------------------\n";

	printl(sep);

	printl("head: %d\n", tty->ibuf_head - tty->ibuf);
	printl("tail: %d\n", tty->ibuf_tail - tty->ibuf);
	printl("cnt: %d\n", tty->ibuf_cnt);

	int pid = tty->tty_caller;
	printl("caller: %s (%d)\n", proc_table[pid].name, pid);
	pid = tty->tty_procnr;
	printl("caller: %s (%d)\n", proc_table[pid].name, pid);

	printl("req_buf: %d\n", (int)tty->tty_req_buf);
	printl("left_cnt: %d\n", tty->tty_left_cnt);
	printl("trans_cnt: %d\n", tty->tty_trans_cnt);

	printl("--------------------------------\n");

	strcpy(sep, "\n");
}

