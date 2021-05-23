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
 *                               ls
 *****************************************************************************/

PUBLIC void ls(char *pathname)
{
    MESSAGE msg;
    msg.type = LS;


	msg.PATHNAME	= (void*)pathname;
	msg.FLAGS	= 0;
	msg.NAME_LEN	= strlen(pathname);

    send_recv(BOTH, TASK_FS, &msg);

    return msg.RETVAL;
}
