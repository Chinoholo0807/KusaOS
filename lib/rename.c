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

PUBLIC int  rename(char *pathname,char *name)
{
	MESSAGE msg;

	msg.type	= RENAME;

	msg.PATHNAME	= (void*)pathname;
	msg.FLAGS	= 0;
	msg.NAME_LEN	= strlen(pathname);
	msg.BUF		= (void*)name;
	msg.BUF_LEN	= strlen(name);

	send_recv(BOTH, TASK_FS, &msg);
	assert(msg.type == SYSCALL_RET);

	return msg.FD;
}

