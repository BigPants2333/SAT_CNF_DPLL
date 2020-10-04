/*
 *  用户交流界面设计 
 */

#ifndef display_h
#define display_h

#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<io.h>
#include<string.h>

#include "basic.h"
#include "cnfparser.h"
#include "solver.h"
#include "binarypuzzle.h"

extern void CNFPage(void);  //CNF文件处理菜单 
extern void binarypuzzlePage(void);  //二进制数独菜单
extern void helpPage(void);  //帮助菜单 

#endif  /*display.h*/
