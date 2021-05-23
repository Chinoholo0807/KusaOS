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

PUBLIC int monitor(){
	u64 long_before,long_change;
	u64 flag = 1;
	int i,base;

	MESSAGE msg;
	msg.type = MONITOR;
	send_recv(BOTH, TASK_MM, &msg);

	if(msg.RETVAL != 0)
	  assert(0);

	long_before = msg.LONG;

	printf("-----------begin--------------\n");

	while(1) {
		msg.type = MONITOR;
		send_recv(BOTH, TASK_MM, &msg);
		long_change = msg.LONG ^ long_before;
		flag = 1;
		long_before = msg.LONG;

		for (i = 0; i < NR_TASKS + NR_PROCS; i++) {
			if(long_change & flag) {
				base = PROCS_BASE + (i - (NR_TASKS + NR_NATIVE_PROCS)) * PROC_IMAGE_SIZE_DEFAULT;
				if(msg.LONG & flag) 
					printf("--PID:%d process on! %08xH-%08xH was allocated--\n",i,base,base+PROC_IMAGE_SIZE_DEFAULT);
				else
					printf("--PID:%d process off! %08xH-%08xH was freed--\n",i,base,base+PROC_IMAGE_SIZE_DEFAULT);
			}
			flag = flag << 1;
		}	
		delay(25);
		//sleep(2);
	}

	return 0;
}
