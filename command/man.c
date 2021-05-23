/*
 * @Description: 
 * @Author: l
 * @Date: 2021-05-05 22:23:53
 * @LastEditors: l
 * @LastEditTime: 2021-05-09 16:20:02
 * @FilePath: \KusaOS-tty.v1\command\man.c
 */
#include "type.h"
#include "stdio.h"
int main(int argc, char * argv[])
{
	printf("   |=========================================================================|\n");
    printf("   |============                    K  U  S  A                    ===========|\n");
    printf("   |=========================================================================|\n");
    printf("   |                                                                         |\n");
    printf("   |************************* COMMANDS FOR SYSTEM ***************************|\n");
    printf("   |                                man : Look up the KusaOS's command       |\n");
    printf("   |                              csche : Change current schedule policy     |\n");
    printf("   |                               kill : Kill process                       |\n");
    printf("   |                                 ps : Show process state                 |\n");
    printf("   |************************* COMMANDS FOR FILE SYSTEM **********************|\n");
    printf("   |                              fs ls : List files in current directory    |\n");
    printf("   |                           fs touch : Create a new file                  |\n");
    printf("   |                              fs rm : Delete a file                      |\n");
    printf("   |                                 cd : Change the directory               |\n");
    printf("   |                           fs mkdir : Create a new directory             |\n");
    printf("   |                                pwd : Print woek directory               |\n");
    printf("   |************************* COMMANDS FOR MEMORY SYSTEM ********************|\n");
    printf("   |                              mstat : show the memory state              |\n");
    printf("   |                             testmm : A test program to fork continually |\n");
    printf("   |                            monitor : start a memory monitor             |\n");
    printf("   |                              buddy : show the buddy system              |\n");
    printf("   |                           testproc : Test for proc                      |\n");
    printf("   |=========================================================================|\n");
	return 0;

}

