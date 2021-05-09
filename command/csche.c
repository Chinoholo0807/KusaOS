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
        pid = argv[1][0]-'0';
        int result = kill_process(pid);
        switch(result){
          case 1:
            printf("Current schedule policy: SHCED_RR\n");
            break;
          case 2:
            printf("Current schedule policy: SHCED_FIFO\n");
            break;
          case 3:
            printf("Current schedule policy: SHCED_PRI\n");
            break;
          case 4:
            printf("Current schedule policy: SHCED_PRI_DY\n");
            break;  
          default:
            printf("Error return result:%d\n",result);
            break;
         }
    }
	return 0;
}
