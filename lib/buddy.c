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

	MESSAGE msg;
	msg.type = BUDDY;

	send_recv(BOTH, TASK_MM, &msg);
	assert(msg.type == SYSCALL_RET);
	return 0;

}
