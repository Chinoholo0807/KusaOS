/*
 * @Description: 
 * @Author: l
 * @Date: 2021-05-05 22:28:56
 * @LastEditors: l
 * @LastEditTime: 2021-05-07 15:38:57
 * @FilePath: \KusaOS\KusaOS\command\testmm.c
 */
#include "type.h"
#include "stdio.h"

//testmm命令
int main(int argc, char * argv[])
{
	int pid = fork();
	if (pid != 0) { //父进程
		printf("parent--pid %d:fork and allocate.\n",getpid());
		int s;
		int i,j,k;
		wait(&s);
		/*for(i=0;i<10;++i)
            		for(j=0;j<10000;++j)
                		for(k=0;k<10;++k)
                {}*/
	}
	else {		//子进程
		int i,j,k;	//等待一段时间，防止在父进程wait()前子进程先退出；
        	for(i=0;i<10;++i)
            		for(j=0;j<10000;++j)
                		for(k=0;k<10;++k)
                {}
		if(getpid()==20)	//防止进程创建过多消耗内存资源
		{
			printf("child --pid 20:exit.\n");
			return 0;
		}
		printf("child --pid %d:execute testmm.\n",getpid());
		exec("testmm");
	}
	return 0;
}
