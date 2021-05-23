/*
 * @Description: 
 * @Author: l
 * @Date: 2021-05-05 22:29:03
 * @LastEditors: l
 * @LastEditTime: 2021-05-10 15:03:34
 * @FilePath: \KusaOS.final.v1\command\testproc.c
 */

#include "type.h"
#include "stdio.h"
#include "string.h"

int main(int argc, char *argv[])
{
	if (argc == 3){
		if (strcmp(argv[1], "loop") == 0){ // testproc loop -- 创建一个一直执行的进程
			change_proc_priority(getpid(), 3);
			int i=1;
			while (1){
				printf("loop..%d\n",i);
				delay(25);
				++i;
			}
			return 0;
		}
		else if (strcmp(argv[1], "sched") == 0){ // testproc sched -- 创建三个进程 A(2) B(3) C(4) 演示对应的调度算法
			int i;
			change_proc_priority(getpid(), 2);
			int pid = fork_priority(3);
			if (pid != 0){
				//process A -- priority 2
				delay(27); // 为了打印工整
				for (i = 0; i < 30; i++){
					printf("A.");
					delay(2);
				}
				printf("--processA finish--\n");
			}
			else{
				pid = fork_priority(4);
				if (pid != 0){
					//process B -- priority 3
					for (i = 0; i < 30; i++){
						printf("B.");
						delay(2);
					}
					printf("--processB finsih--.\n");
				}
				else{ // process C -- priority 4
					for (i = 0; i < 30; i++){
						printf("C.");
						delay(2);
					}
					printf("--processC finish--.\n");
				}
			}
			return 0;
		}else if(strcmp(argv[1],"normal")==0){ // testproc normal -- 创建一个正常进程，缓慢打印
			change_proc_priority(getpid(), 3);
			int i;
			printf("exit when number = 10\n");
			for(i=1;i<=10;++i){
				delay(25);
				printf("normal...%d\n",i);
			}
			printf("exit...\n");

		}
	}
	return 0;
}
