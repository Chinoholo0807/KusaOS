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
	if(argc>1&&argv[1][0]=='1')
	{
		mstat(getpid());
	}
	else if(argc>1&&argv[1][0]=='2')
	{
		totalmm();
		//printl("2 -- print total memory size\n");
	}
	else if(argc>1&&argv[1][0]=='3')	//测试函数：双进程
	{
		int pid = fork();
		if (pid != 0) { //父进程
			printl("parent--pid %d:fork and allocate.\n",getpid());
			int s;
			wait(&s);
		}
		else {		//子进程
			printl("child--pid %d:execute testmm\n",getpid());
			argv[1][0]='1';		//下次执行功能1
			execv("testmm",argv);
			//execv("showmm","hhhhhhh");
		}
	}
	else if(argc>1&&argv[1][0]=='4')	//buddy测试
	{
		buddy();
	}
	else		//其它情况：打印帮助信息
	{
		printl("<Usage>: [testmm] [number]\n");
		printl("number:\n");
		printl("1 -- print current process's pid and memory baseaddr\n");
		printl("2 -- print total memory size\n");
		printl("3 -- fork a child process to show the infomation of allocation\n");
		printl("4 -- a test for buddy system");
		printl("others -- show this help infomation\n");
		
	}

	return 0;

}
