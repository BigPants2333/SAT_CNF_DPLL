/*
 *  求解器算法理论概要： 
 *  1-变量决策策略：
 *          1）-最短子句出现频率最大优先即MOM 
 *          2）-独立变量衰减和策略即VSIDS策略 
 *          3）-由第一个子句的第一个文字开始决策
 *          4）-从出现频率最高的文字开始决策
 *          5）-对子句进行加权操作以进行决策 
 *  2-回溯时引入学习子句以解决冲突 
 *  3-子句删除机制 
 *  4-随机重启动机制
 */ 

#ifndef solver_h
#define solver_h

#include "basic.h"
#include <math.h>

#define CLAUSE 0
#define LITERAL 1
#define SPLIT 2

//===========================================================================================
//辅助结构及变量 

typedef struct Stack {
    int tag;
    int floor;
    Literal *Lp;
    Clause *Cp;
    struct Stack *next;
} Stack;

typedef struct Floor {
    int a;
    int floor;
} Floor;

enum strategy{
	mom,
	vsids,
	first,
	frequency,
	weight_s,
};

//===========================================================================================
//策略相关函数

extern int MOM_Strategy(CNF_Solver *cnf);  //MOM策略
extern int VSIDS_Strategy(CNF_Solver *cnf);  //VSIDS策略
extern int Frequency_Strategy(CNF_Solver *cnf);  //频率策略
extern int Weight_Strategy(CNF_Solver *cnf);  //权重策略
extern int Combine_Strategy(CNF_Solver *cnf, enum strategy s);  //综合策略

//===========================================================================================
//学习子句相关函数

extern status Recover_Change(CNF_Solver *cnf, Stack *head, int time, int *learnarray);//恢复修改
extern int Create_LearnClause(CNF_Solver *cnf, int *a, int i, int X);  //生成学习子句
extern status Backtrack_LearnClause(CNF_Solver *cnf, int floor);  //学习子句的回溯
extern status Delete_Repeat_LearnClause(CNF_Solver *cnf, LearnClause *Lcp);  //删去重复的学习子句

//===========================================================================================
//子句化简相关函数

extern status Simplify_UnitClause_1(CNF_Solver *cnf, int literal, Stack *head);//优化前单子句化简
extern status Simplify_UnitClause_2(CNF_Solver *cnf, Clause *Cp, Stack *head);//优化后单子句化简

//===========================================================================================
//辅助函数

extern status Save_Change(Stack *head, int tag, Clause* Cp, Literal *Lp);//保存化简时的修改
extern status Check_Res(CNF_Solver *cnf);  //检查答案
extern status Print_Stack(Stack *head);  //打印栈
extern status Backtrack_Assign(CNF_Solver *cnf, int floor);  //赋值回溯
extern int Compare_Des(const void *a,const void *b);  //决策层差值

//===========================================================================================
//DPLL递归求解函数

extern boolean DPLL_Rec_1(CNF_Solver *cnf, const int f);  //递归，优化前
extern boolean DPLL_Rec_2(CNF_Solver *cnf,const int f);  //递归，优化后

#endif // DPLL_H_INCLUDED
