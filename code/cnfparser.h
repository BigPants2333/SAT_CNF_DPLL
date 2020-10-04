/*
 *  对cnf文件进行分析处理 
 *  将数据写入求解器中
 *  保存求解出的解 
 */

#ifndef cnfparser_h
#define cnfparser_h

#include "basic.h"

#define LITERALMAX 200

typedef char status;

extern CNF_Solver* Load_CNF(char *path);  //读取数据
extern status To_Save_Path(char *savepath, char *cnfpath);  //变换后缀名res
extern status Save_Res(char *respath, status DPLL, CNF_Solver *cnf, int time);  //保存结果
extern status Save_CNF(CNF_Solver *cnf,char *path);  //保存数据

#endif  /*cnfparser.h*/
