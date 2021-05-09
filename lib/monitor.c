

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
	int i,j,k;

	MESSAGE msg;
	msg.type = MONITOR;
	send_recv(BOTH, TASK_MM, &msg);

	if(msg.RETVAL != 0)
	  assert(0);

	long_before = msg.LONG;

	printl("-----------%d--------------\n",long_before);

	while(1) {
		msg.type = MONITOR;
		send_recv(BOTH, TASK_MM, &msg);
		long_change = msg.LONG ^ long_before;
		flag = 1;
		long_before = msg.LONG;

		for (i = 0; i < NR_TASKS + NR_PROCS; i++) {
			if(long_change & flag)
				if(msg.LONG & flag)
					printf("-----------process on%d-----------\n",i);
				else
					printf("-----------process off%d-----------\n",i);
			flag = flag << 1;
		}	
		for(j = 0; j<=999999;j++);
		
		
	}

	return 0;
}
