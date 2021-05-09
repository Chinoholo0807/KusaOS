/*
 * @Description: 
 * @Author: l
 * @Date: 2021-05-08 11:16:38
 * @LastEditors: l
 * @LastEditTime: 2021-05-08 12:44:12
 * @FilePath: \KusaOS-tty.v1\command\kill.c
 */
#include "type.h"
#include "stdio.h"

int main(int argc, char * argv[])
{
    if(argc !=2){
        printf("args error,pleace input [ csche NR_POLICY ]\n");
        printf("1 -- SCHED_RR  2 -- SCHED_FIFO\n");
        printf("3 -- SCHED_PRI 4 -- SCHED_PRI_DY\n");
    }else{
        int nr_policy =0;
        nr_policy = argv[1][0]-'0';
        int result = change_schedule_policy(nr_policy);
        char *schedule_name[]={"","SCHED_RR","SCHED_FIFO","SCHED_PRI","SCHED_PRI_DY"}
	if( result>0&& result<=4)
		printf("Current schedule policy:%s\n",schedule_name[reslut]);
	else
		printf("Error,change schedule policy failed\n");
    }
	return 0;
}
