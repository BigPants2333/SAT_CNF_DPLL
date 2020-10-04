/*
 *  定义文字、子句、求解器的物理结构 
 *  宏定义意义字符的值和常用函数 
 *  定义文字、子句、求解器相关的最小函数操作 
 */
 
#ifndef basic_h
#define basic_h

#include <stdio.h>
#include <stdlib.h>

#define OK 1
#define ERROR 0

#define TRUE 1
#define FALSE -1
#define NOTSURE 0
#define NOTCONTAIN 2

#define ASSIGN(x) ((x>0)?1:-1)
#define ABS(x) ((x>0)?(x):0-(x))

typedef char boolean;
typedef char status;

//===========================================================================================
//存储结构 

typedef struct Literal {  //文字结构 
    int literal;  //文字，正为原形式，负为非形式
    struct Literal *next;  //指向下一个文字
} Literal;

typedef struct Clause {  //子句结构 
    status isremoved;  //子句是否被移出子句集
    int weight;  //权重
    int length;  //子句包含文字的数目
    struct Literal *head;  //指向子句中第一个文字
    struct Literal *rmv;  //指向子句中第一个被移除的文字
    struct Clause *prev;  //指向上一个子句
    struct Clause *next;  //指向下一个子句
} Clause;

typedef struct LearnClause {  //学习子句结构 
    boolean isInStack;  //是否在回溯栈中
    int floor;  //决策级数
    int call_num;  //调用次数
    struct Clause *clause;  //指向学习子句
    struct LearnClause *next;  //指向下一个学习子句
} LearnClause;

typedef struct LiteralIndex {  //文字索引结构 
    Literal *Lp;  //指向与其下标一致的文字
    Clause *Cp;  //指向文字出现的子句
    struct LiteralIndex *next;  //下一个文字
} LiteralIndex;

typedef struct Solver {  //求解器结构 
    int literal_num;  //文字个数
    int clause_num;  //子句个数
    int *floorarray;  //文字所在的决策层
    int *countarray;  //VSIDS决策数组
    boolean *boolarray;  //文字真假赋值数组，-1为假，1为真，0为未知 
    struct LiteralIndex *lindex;    //指向文字索引
    struct Clause *root;  //指向第一个子句
    struct LearnClause *learn_root;  //指向第一个学习子句
} CNF_Solver,BP_Solver;

//===========================================================================================
//文字相关函数 

extern status Add_Literal(Clause *Cp,int literal);  //增加文字 
extern Literal* Remove_Literal(Clause *Cp, int literal);  //移除文字 
extern status Recover_Literal(Clause *Cp, Literal *Lp);  //恢复文字 
extern status Delete_Literal(Clause *Cp, int literal);  //删除文字 

extern status Boolarray_Assign(CNF_Solver *cnf, int literal, int f);  //保存到数组

//===========================================================================================
//文字索引相关函数

extern status Create_Literal_Index(CNF_Solver *cnf);  //创建文字索引
extern status Add_Literal_Index(CNF_Solver *cnf, Clause *Cp);  //添加文字索引
extern status Delete_Clause_Literal_Index(CNF_Solver *cnf, Clause *Cp);  //删除子句的文字索引
extern status Delete_All_Literal_Index(CNF_Solver *cnf);  //删除所有文字索引

extern status Print_Literal_Index(CNF_Solver *cnf);  //打印文字索引

//===========================================================================================
//子句相关函数 

extern status Add_Clause(CNF_Solver *cnf, int n, int *clause);  //增加子句
extern status Insert_Clause_Top(CNF_Solver *cnf, Clause *Cp);  //插入子句在顶部

extern Clause* Remove_Clause(CNF_Solver *cnf, Clause *Cp);  //移除子句
extern status Delete_Clause(CNF_Solver *cnf, Clause *Cp);  //删除子句
extern status Delete_All_Clause(CNF_Solver *cnf);  //删除所有子句
extern status Delete_LearnClause(CNF_Solver *cnf, LearnClause *Lcp);  //删除学习子句

extern status Print_Clause(CNF_Solver *cnf);  //打印子句
extern status Print_LearnClause(CNF_Solver *cnf);  //打印学习子句

extern boolean Is_Clause_Empty(CNF_Solver *cnf);  //子句判空
extern boolean Is_Unit_Clause(Clause *Cp);  //是否为单子句
extern boolean Evaluate_Clause(Clause *Cp, int literal);  //子句判真
extern boolean Have_Empty_Clause(CNF_Solver *cnf);  //是否有空子句
extern boolean Have_Clause(CNF_Solver *cnf);  //子句集是否为空

extern Clause* Find_Unit_Clause(CNF_Solver *cnf);  //是否有单子句

//===========================================================================================
//求解器相关函数

extern status Create_Solver(struct Solver *solver, int literal_num, int clause_num);//Solver创建
extern status Destroy_Solver(struct Solver *solver);  //Solver销毁
extern status Clear_Solver_Answer(struct Solver *solver);  //Solver清空答案

#endif  /*basic.h*/
