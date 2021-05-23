/*
 * @Description: 
 * @Author: l
 * @Date: 2021-05-08 15:35:36
 * @LastEditors: l
 * @LastEditTime: 2021-05-08 15:50:40
 * @FilePath: \KusaOS-tty.v1\lib\rand.c
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
/* random seed */
PRIVATE unsigned int _seed2 =0xDEADBEEF;
void srand(unsigned int seed){
	_seed2 = seed;
}

int rand() {
	int next = _seed2;
	int result;

	next *= 1103515245;
	next += 12345;
	result = (next / 65536) ;

	next *= 1103515245;
	next += 12345;
	result <<= 10;
	result ^= (next / 65536) ;

	next *= 1103515245;
	next += 12345;
	result <<= 10;
	result ^= (next / 65536) ;

	_seed2 = next;

	return result>0 ? result : -result;
}