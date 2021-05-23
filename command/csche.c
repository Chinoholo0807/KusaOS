/*

 * @Description: 

 * @Author: l

 * @Date: 2021-05-08 11:16:38

 * @LastEditors: l

 * @LastEditTime: 2021-05-10 13:17:16

 * @FilePath: \KusaOS_@@\command\csche.c

 */

#include "type.h"

#include "stdio.h"

int main(int argc, char *argv[])
{
    char *schedule_name[] = {"", "SCHED_RR", "SCHED_PRI", "SCHED_FIFO", "SCHED_PRI_DY"};
    if (argc == 2)
    {
        printf("[ csche NR_POLICY ]\n");
        printf("1 -- SCHED_RR \n");
        printf("2 -- SCHED_PRI \n");
        int result = change_schedule_policy(-1);
        printf("Current schedule policy:%s\n", schedule_name[result]);
        return 0;
    }
    if (argc != 3)
    {
        printf("args error,pleace input [ csche NR_POLICY ]\n");
        printf("1 -- SCHED_RR \n");
        printf("2 -- SCHED_PRI \n");
    }
    else
    {
        int nr_policy = 0;
        nr_policy = argv[1][0] - '0';
        //change_proc_priority(getpid(),100);
        int result = change_schedule_policy(nr_policy);
        if (result > 0 && result <= 4)
            printf("Current schedule policy:%s\n", schedule_name[result]);
        else
            printf("Error,change schedule policy failed，current schedule policy:%s\n", schedule_name[result]);
    }
    return 0;
}