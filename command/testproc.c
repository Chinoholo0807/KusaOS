/*
 * @Description: 
 * @Author: l
 * @Date: 2021-05-05 22:29:03
 * @LastEditors: l
 * @LastEditTime: 2021-05-05 22:29:03
 * @FilePath: \KusaOS\KusaOS\command\testproc.c
 */
#include "type.h"
#include "stdio.h"

int main(int argc, char * argv[])
{
	int i;
	for (i = 1; i < argc; i++)
		printf("%s%s", i == 1 ? "" : " ", argv[i]);
	printf("\n");
	int ret = change_schedule_policy(4);
		printf("current scheduel policy:%d",ret);
	change_proc_priority(5,10);
	change_proc_priority(6,11);
	change_proc_priority(7,12);
	return 0;
}
