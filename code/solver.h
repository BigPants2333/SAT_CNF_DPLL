/*
 *  ������㷨���۸�Ҫ�� 
 *  1-�������߲��ԣ�
 *          1��-����Ӿ����Ƶ��������ȼ�MOM 
 *          2��-��������˥���Ͳ��Լ�VSIDS���� 
 *          3��-�ɵ�һ���Ӿ�ĵ�һ�����ֿ�ʼ����
 *          4��-�ӳ���Ƶ����ߵ����ֿ�ʼ����
 *          5��-���Ӿ���м�Ȩ�����Խ��о��� 
 *  2-����ʱ����ѧϰ�Ӿ��Խ����ͻ 
 *  3-�Ӿ�ɾ������ 
 *  4-�������������
 */ 

#ifndef solver_h
#define solver_h

#include "basic.h"
#include <math.h>

#define CLAUSE 0
#define LITERAL 1
#define SPLIT 2

//===========================================================================================
//�����ṹ������ 

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
//������غ���

extern int MOM_Strategy(CNF_Solver *cnf);  //MOM����
extern int VSIDS_Strategy(CNF_Solver *cnf);  //VSIDS����
extern int Frequency_Strategy(CNF_Solver *cnf);  //Ƶ�ʲ���
extern int Weight_Strategy(CNF_Solver *cnf);  //Ȩ�ز���
extern int Combine_Strategy(CNF_Solver *cnf, enum strategy s);  //�ۺϲ���

//===========================================================================================
//ѧϰ�Ӿ���غ���

extern status Recover_Change(CNF_Solver *cnf, Stack *head, int time, int *learnarray);//�ָ��޸�
extern int Create_LearnClause(CNF_Solver *cnf, int *a, int i, int X);  //����ѧϰ�Ӿ�
extern status Backtrack_LearnClause(CNF_Solver *cnf, int floor);  //ѧϰ�Ӿ�Ļ���
extern status Delete_Repeat_LearnClause(CNF_Solver *cnf, LearnClause *Lcp);  //ɾȥ�ظ���ѧϰ�Ӿ�

//===========================================================================================
//�Ӿ仯����غ���

extern status Simplify_UnitClause_1(CNF_Solver *cnf, int literal, Stack *head);//�Ż�ǰ���Ӿ仯��
extern status Simplify_UnitClause_2(CNF_Solver *cnf, Clause *Cp, Stack *head);//�Ż����Ӿ仯��

//===========================================================================================
//��������

extern status Save_Change(Stack *head, int tag, Clause* Cp, Literal *Lp);//���滯��ʱ���޸�
extern status Check_Res(CNF_Solver *cnf);  //����
extern status Print_Stack(Stack *head);  //��ӡջ
extern status Backtrack_Assign(CNF_Solver *cnf, int floor);  //��ֵ����
extern int Compare_Des(const void *a,const void *b);  //���߲��ֵ

//===========================================================================================
//DPLL�ݹ���⺯��

extern boolean DPLL_Rec_1(CNF_Solver *cnf, const int f);  //�ݹ飬�Ż�ǰ
extern boolean DPLL_Rec_2(CNF_Solver *cnf,const int f);  //�ݹ飬�Ż���

#endif // DPLL_H_INCLUDED
