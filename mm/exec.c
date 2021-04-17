/*****************************************************************************
 *****************************************************************************
 * KusaOS/mm/exec.c
 * Last modification: by llm,2021/4/17
 * Function: 实现系统调用exec()

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
#include "elf.h"


/*****************************************************************************
 *****************************************************************************
 * do_exec()
 * 功能: 执行系统调用exec()
 * 返回值：调用成功返回0，失败返回-1
 * 1. 从消息体中获取各种参数。由于调用者和MM处在不同的地址空间，所以对于文件名这样的一段内存，需要通过获取其物理地址并进行物理地址复制。
 * 2. 通过一个新的系统调用stat( )获取被执行文件的大小。
 * 3. 将被执行文件全部读入MM自己的缓冲区（MM的缓冲区有1MB，我们姑且假设这个空间足够了。等有一天真的不够了，会触发一个assert，到时我们再做打算）。
 * 4. 根据ELF文件的程序头（Program Header）信息，将被执行文件的各个段放置到合适的位置。
 * 5. 建立参数栈──这个栈在execv( )中已经准备好了，但由于内存空间发生了变化，所以里面所有的指针都需要重新定位，这个过程并不难，通过一个delta变量即可完成。
 * 6. 为被执行程序的eax和ecx赋值。
 * 7. 为程序的eip赋值，这是程序的入口地址，即_start处。
 * 8. 为程序的esp赋值。注意要闪出刚才我们准备好的堆栈的位置。
 * 9. 最后是将进程的名字改成被执行程序的名字。

 *****************************************************************************
 *****************************************************************************/

PUBLIC int do_exec()
{
	/* 从mm_msg中获取参数 */
	int name_len = mm_msg.NAME_LEN;	/* 文件名长度 */
	int src = mm_msg.source;	
	assert(name_len < MAX_PATH);

	char pathname[MAX_PATH];
	phys_copy((void*)va2la(TASK_MM, pathname),
		  (void*)va2la(src, mm_msg.PATHNAME),
		  name_len);
	pathname[name_len] = 0;	

	/* 获取被执行文件大小 */
	struct stat s;
	int ret = stat(pathname, &s);
	if (ret != 0) {
		printl("{MM} MM::do_exec()::stat() returns error. %s", pathname);
		return -1;
	}

	/* 将被执行文件读入MM的缓冲区 */
	int fd = open(pathname, O_RDWR);
	if (fd == -1)
		return -1;
	assert(s.st_size < MMBUF_SIZE);
	read(fd, mmbuf, s.st_size);
	close(fd);

	/* 将当前的进程映像替换为新的 */
	Elf32_Ehdr* elf_hdr = (Elf32_Ehdr*)(mmbuf);
	int i;
	for (i = 0; i < elf_hdr->e_phnum; i++) {
		Elf32_Phdr* prog_hdr = (Elf32_Phdr*)(mmbuf + elf_hdr->e_phoff +
			 			(i * elf_hdr->e_phentsize));
		if (prog_hdr->p_type == PT_LOAD) {
			assert(prog_hdr->p_vaddr + prog_hdr->p_memsz <
				PROC_IMAGE_SIZE_DEFAULT);
			phys_copy((void*)va2la(src, (void*)prog_hdr->p_vaddr),
				  (void*)va2la(TASK_MM,
						 mmbuf + prog_hdr->p_offset),
				  prog_hdr->p_filesz);
		}
	}

	/* 设置参数栈 */
	int orig_stack_len = mm_msg.BUF_LEN;
	char stackcopy[PROC_ORIGIN_STACK];
	phys_copy((void*)va2la(TASK_MM, stackcopy),
		  (void*)va2la(src, mm_msg.BUF),
		  orig_stack_len);

	u8 * orig_stack = (u8*)(PROC_IMAGE_SIZE_DEFAULT - PROC_ORIGIN_STACK);

	int delta = (int)orig_stack - (int)mm_msg.BUF;

	int argc = 0;
	if (orig_stack_len) {	
		char **q = (char**)stackcopy;
		for (; *q != 0; q++,argc++)
			*q += delta;
	}

	phys_copy((void*)va2la(src, orig_stack),
		  (void*)va2la(TASK_MM, stackcopy),
		  orig_stack_len);

	
	proc_table[src].regs.ecx = argc; /* argc */
	proc_table[src].regs.eax = (u32)orig_stack; /* argv */

	/* 设置寄存器 eip 和 esp */
	proc_table[src].regs.eip = elf_hdr->e_entry; 
	proc_table[src].regs.esp = PROC_IMAGE_SIZE_DEFAULT - PROC_ORIGIN_STACK;

	/* 将进程的名字改成被执行程序的名字 */
	strcpy(proc_table[src].name, pathname);

	return 0;
}
