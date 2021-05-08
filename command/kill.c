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
        printf("args error,pleace input [ kill pid ]\n");
    }else{
        int pid =0;
        pid = argv[1][0]-'0';
        if(argv[1][1]<='9'&&argv[1][1]>='0')
            pid =pid*10 + argv[1][1]-'0';
        if(pid == getpid()){
            printf("Can't kill yourself!\n");
            return 0;
        }
        int result = kill_process(pid);
        if(result==0)
            printf("Kill process %d success\n",pid);
        else
            printf("Kill process %d fail\n",pid);
    }
	return 0;
}
