/*
 * @Description: 
 * @Author: l
 * @Date: 2021-05-09 16:18:16
 * @LastEditors: l
 * @LastEditTime: 2021-05-09 16:20:25
 * @FilePath: \KusaOS-tty.v1\command\echo.c
 */
#include "stdio.h"

int main(int argc, char * argv[])
{
	int i;
	for (i = 1; i < argc; i++)
		printf("%s%s", i == 1 ? "" : " ", argv[i]);
	printf("\n");
	return 0;
}
