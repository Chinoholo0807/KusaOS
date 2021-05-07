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



PUBLIC int is_dir(const char *pathname)
{
        MESSAGE msg;

        msg.type        = IS_DIR;

        msg.PATHNAME    = (void*)pathname;
        msg.NAME_LEN    = strlen(pathname);

        send_recv(BOTH, TASK_FS, &msg);
        assert(msg.type == SYSCALL_RET);

        return msg.RETVAL;
}
