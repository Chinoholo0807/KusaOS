/*
 * @Description: 
 * @Author: l
 * @Date: 2021-05-06 23:29:45
 * @LastEditors: l
 * @LastEditTime: 2021-05-07 00:05:42
 * @FilePath: \KusaOS-mid\lib\buddy.c
 */


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

PUBLIC int buddy()
{	
	int i;

	MESSAGE msg;
	msg.type = BUDDY;

	send_recv(BOTH, TASK_MM, &msg);
	assert(msg.type == SYSCALL_RET);
	if(msg.RETVAL != 0)
	  	assert(0);

	for(i=1;i<=10;i++)
	{
		printf("size: %2dK     ",msg.LONG);
		printf("addr: %08xH\n",msg.LONG2);		
		msg.type = BUDDY;
		send_recv(BOTH, TASK_MM, &msg);
	}
	return 0;

}
