/*
 *  �������֡��Ӿ䡢�����������ṹ 
 *  �궨�������ַ���ֵ�ͳ��ú��� 
 *  �������֡��Ӿ䡢�������ص���С�������� 
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
//�洢�ṹ 

typedef struct Literal {  //���ֽṹ 
    int literal;  //���֣���Ϊԭ��ʽ����Ϊ����ʽ
    struct Literal *next;  //ָ����һ������
} Literal;

typedef struct Clause {  //�Ӿ�ṹ 
    status isremoved;  //�Ӿ��Ƿ��Ƴ��Ӿ伯
    int weight;  //Ȩ��
    int length;  //�Ӿ�������ֵ���Ŀ
    struct Literal *head;  //ָ���Ӿ��е�һ������
    struct Literal *rmv;  //ָ���Ӿ��е�һ�����Ƴ�������
    struct Clause *prev;  //ָ����һ���Ӿ�
    struct Clause *next;  //ָ����һ���Ӿ�
} Clause;

typedef struct LearnClause {  //ѧϰ�Ӿ�ṹ 
    boolean isInStack;  //�Ƿ��ڻ���ջ��
    int floor;  //���߼���
    int call_num;  //���ô���
    struct Clause *clause;  //ָ��ѧϰ�Ӿ�
    struct LearnClause *next;  //ָ����һ��ѧϰ�Ӿ�
} LearnClause;

typedef struct LiteralIndex {  //���������ṹ 
    Literal *Lp;  //ָ�������±�һ�µ�����
    Clause *Cp;  //ָ�����ֳ��ֵ��Ӿ�
    struct LiteralIndex *next;  //��һ������
} LiteralIndex;

typedef struct Solver {  //������ṹ 
    int literal_num;  //���ָ���
    int clause_num;  //�Ӿ����
    int *floorarray;  //�������ڵľ��߲�
    int *countarray;  //VSIDS��������
    boolean *boolarray;  //������ٸ�ֵ���飬-1Ϊ�٣�1Ϊ�棬0Ϊδ֪ 
    struct LiteralIndex *lindex;    //ָ����������
    struct Clause *root;  //ָ���һ���Ӿ�
    struct LearnClause *learn_root;  //ָ���һ��ѧϰ�Ӿ�
} CNF_Solver,BP_Solver;

//===========================================================================================
//������غ��� 

extern status Add_Literal(Clause *Cp,int literal);  //�������� 
extern Literal* Remove_Literal(Clause *Cp, int literal);  //�Ƴ����� 
extern status Recover_Literal(Clause *Cp, Literal *Lp);  //�ָ����� 
extern status Delete_Literal(Clause *Cp, int literal);  //ɾ������ 

extern status Boolarray_Assign(CNF_Solver *cnf, int literal, int f);  //���浽����

//===========================================================================================
//����������غ���

extern status Create_Literal_Index(CNF_Solver *cnf);  //������������
extern status Add_Literal_Index(CNF_Solver *cnf, Clause *Cp);  //�����������
extern status Delete_Clause_Literal_Index(CNF_Solver *cnf, Clause *Cp);  //ɾ���Ӿ����������
extern status Delete_All_Literal_Index(CNF_Solver *cnf);  //ɾ��������������

extern status Print_Literal_Index(CNF_Solver *cnf);  //��ӡ��������

//===========================================================================================
//�Ӿ���غ��� 

extern status Add_Clause(CNF_Solver *cnf, int n, int *clause);  //�����Ӿ�
extern status Insert_Clause_Top(CNF_Solver *cnf, Clause *Cp);  //�����Ӿ��ڶ���

extern Clause* Remove_Clause(CNF_Solver *cnf, Clause *Cp);  //�Ƴ��Ӿ�
extern status Delete_Clause(CNF_Solver *cnf, Clause *Cp);  //ɾ���Ӿ�
extern status Delete_All_Clause(CNF_Solver *cnf);  //ɾ�������Ӿ�
extern status Delete_LearnClause(CNF_Solver *cnf, LearnClause *Lcp);  //ɾ��ѧϰ�Ӿ�

extern status Print_Clause(CNF_Solver *cnf);  //��ӡ�Ӿ�
extern status Print_LearnClause(CNF_Solver *cnf);  //��ӡѧϰ�Ӿ�

extern boolean Is_Clause_Empty(CNF_Solver *cnf);  //�Ӿ��п�
extern boolean Is_Unit_Clause(Clause *Cp);  //�Ƿ�Ϊ���Ӿ�
extern boolean Evaluate_Clause(Clause *Cp, int literal);  //�Ӿ�����
extern boolean Have_Empty_Clause(CNF_Solver *cnf);  //�Ƿ��п��Ӿ�
extern boolean Have_Clause(CNF_Solver *cnf);  //�Ӿ伯�Ƿ�Ϊ��

extern Clause* Find_Unit_Clause(CNF_Solver *cnf);  //�Ƿ��е��Ӿ�

//===========================================================================================
//�������غ���

extern status Create_Solver(struct Solver *solver, int literal_num, int clause_num);//Solver����
extern status Destroy_Solver(struct Solver *solver);  //Solver����
extern status Clear_Solver_Answer(struct Solver *solver);  //Solver��մ�

#endif  /*basic.h*/
