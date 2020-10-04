/*
 *  二进制数独限制规则： 
 *    1-每一行、每一列中没有连续的三个1或0
 *    2-每一行、每一列中0和1的个数相同
 *    3-没有重复的行或列 
 */
 
#ifndef binarypuzzle_h
#define binarypuzzle_h

#include<string.h>
#include "solver.h"

//===========================================================================================
//处理文件中BP字符串结构体 

typedef struct BP_NODE{
	int count;  //字符串顺序号 
	char *s;  //字符串 
	struct BP_NODE *next;  //指向下一个结点 
}BP_node;

typedef struct BP_FILE{
	int n;  //二进制数独阶数 
	int num;  //二进制数独个数 
	BP_node *head;  //指向第一个结点 
}BP_file;

//===========================================================================================
//对合法棋盘挖洞使用的结构体

typedef struct HoleNode{
	int val;  //洞的值1为真，0为假 
	int mark;  //标志是否挖去，0为不挖，1为挖去 
}Hole; 

//===========================================================================================
//限制条件子句生成

extern status BP_Constraint_1(BP_Solver *bp,int n);  //生成限制规则一子句 
extern status BP_Constraint_2(BP_Solver *bp,int n);  //生成限制规则二子句 
extern status BP_Constraint_3(BP_Solver *bp,int n);  //生成限制规则三子句 

extern BP_Solver *Create_BP_CNF(int n);  //生成限制规则子句 

//===========================================================================================
//处理文件中BP字符串函数

extern BP_file* Read_BP_File(char *path);  //读取文件中的信息构成结构网络 
extern status Read_BP_Str(BP_Solver *bp,int n,char *s);  //根据字符串中的真假情况添加子句 
extern char* Solved_BP_Str(BP_Solver *bp,int n);  //将解转化为字符串 

//===========================================================================================
//生成随机初始棋盘函数 

extern status Two_Random_Cells(BP_Solver *bp,int n);  //随机对两个格子赋值 
extern char* Initialize_BP(BP_Solver *bp,int n);  //生成初始完整棋盘 
extern char* Dig_Holes(char *initial,int n);  //挖洞 
extern BP_Solver* Create_Random_BP(BP_Solver *bp,int n,char *s);  //产生随机棋盘 

//===========================================================================================
//辅助函数

extern status Print_BP_Str(char *bp_str,int n);  //打印棋盘字符串 

extern int Fac(int n);  //求阶乘 
extern int Com(int n,int m);  //求组合数
extern void IterativeCombos(int n,int m,int C_Arr[],int E_Arr[],int index[]); //求所有组合可能 
 

#endif  /*binarypuzzle.h*/ 
