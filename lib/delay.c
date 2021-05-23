/*
 * @Description: 
 * @Author: l
 * @Date: 2021-05-09 19:38:47
 * @LastEditors: l
 * @LastEditTime: 2021-05-10 10:37:32
 * @FilePath: \KusaOS_@@\lib\delay.c
 */

#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"
PUBLIC void delay(int milli_sec)
{
        int i,j,k;
        for(i=0;i<milli_sec;++i)
            for(j=0;j<10000;++j)
                for(k=0;k<10;++k)
                {}

}

