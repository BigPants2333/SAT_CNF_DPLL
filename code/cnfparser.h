/*
 *  ��cnf�ļ����з������� 
 *  ������д���������
 *  ���������Ľ� 
 */

#ifndef cnfparser_h
#define cnfparser_h

#include "basic.h"

#define LITERALMAX 200

typedef char status;

extern CNF_Solver* Load_CNF(char *path);  //��ȡ����
extern status To_Save_Path(char *savepath, char *cnfpath);  //�任��׺��res
extern status Save_Res(char *respath, status DPLL, CNF_Solver *cnf, int time);  //������
extern status Save_CNF(CNF_Solver *cnf,char *path);  //��������

#endif  /*cnfparser.h*/
