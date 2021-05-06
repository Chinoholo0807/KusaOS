/*
 * @Description: 
 * @Author: l
 * @Date: 2021-05-05 22:23:53
 * @LastEditors: l
 * @LastEditTime: 2021-05-06 20:36:55
 * @FilePath: \KusaOS-mid\command\man.c
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
    printf("   |                            +  man  : Look up the KusaOS's command       |\n");
    printf("   |                              clear : Clear the screen                   |\n");
    printf("   |                            +   ps : Show process state                 |\n");
    printf("   |************************* COMMANDS FOR FILE SYSTEM **********************|\n");
    printf("   |                                 ls : List files in current directory    |\n");
    printf("   |                              touch : Create a new file                  |\n");
    printf("   |                                cat : View a file                        |\n");
    printf("   |                                 vi : Modify a file                      |\n");
    printf("   |                                 rm : Delete a file                      |\n");
    printf("   |                                 cd : Change the directory               |\n");
    printf("   |                              mkdir : Create a new directory             |\n");
    printf("   |************************* COMMANDS FOR TEST *****************************|\n");
    printf("   |                             testmm : Test for MM                        |\n");
    printf("   |                           testproc : Test for proc                      |\n");
    printf("   |=========================================================================|\n");
	return 0;
}

