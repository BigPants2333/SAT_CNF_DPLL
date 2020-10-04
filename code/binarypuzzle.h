/*
 *  �������������ƹ��� 
 *    1-ÿһ�С�ÿһ����û������������1��0
 *    2-ÿһ�С�ÿһ����0��1�ĸ�����ͬ
 *    3-û���ظ����л��� 
 */
 
#ifndef binarypuzzle_h
#define binarypuzzle_h

#include<string.h>
#include "solver.h"

//===========================================================================================
//�����ļ���BP�ַ����ṹ�� 

typedef struct BP_NODE{
	int count;  //�ַ���˳��� 
	char *s;  //�ַ��� 
	struct BP_NODE *next;  //ָ����һ����� 
}BP_node;

typedef struct BP_FILE{
	int n;  //�������������� 
	int num;  //�������������� 
	BP_node *head;  //ָ���һ����� 
}BP_file;

//===========================================================================================
//�ԺϷ������ڶ�ʹ�õĽṹ��

typedef struct HoleNode{
	int val;  //����ֵ1Ϊ�棬0Ϊ�� 
	int mark;  //��־�Ƿ���ȥ��0Ϊ���ڣ�1Ϊ��ȥ 
}Hole; 

//===========================================================================================
//���������Ӿ�����

extern status BP_Constraint_1(BP_Solver *bp,int n);  //�������ƹ���һ�Ӿ� 
extern status BP_Constraint_2(BP_Solver *bp,int n);  //�������ƹ�����Ӿ� 
extern status BP_Constraint_3(BP_Solver *bp,int n);  //�������ƹ������Ӿ� 

extern BP_Solver *Create_BP_CNF(int n);  //�������ƹ����Ӿ� 

//===========================================================================================
//�����ļ���BP�ַ�������

extern BP_file* Read_BP_File(char *path);  //��ȡ�ļ��е���Ϣ���ɽṹ���� 
extern status Read_BP_Str(BP_Solver *bp,int n,char *s);  //�����ַ����е�����������Ӿ� 
extern char* Solved_BP_Str(BP_Solver *bp,int n);  //����ת��Ϊ�ַ��� 

//===========================================================================================
//���������ʼ���̺��� 

extern status Two_Random_Cells(BP_Solver *bp,int n);  //������������Ӹ�ֵ 
extern char* Initialize_BP(BP_Solver *bp,int n);  //���ɳ�ʼ�������� 
extern char* Dig_Holes(char *initial,int n);  //�ڶ� 
extern BP_Solver* Create_Random_BP(BP_Solver *bp,int n,char *s);  //����������� 

//===========================================================================================
//��������

extern status Print_BP_Str(char *bp_str,int n);  //��ӡ�����ַ��� 

extern int Fac(int n);  //��׳� 
extern int Com(int n,int m);  //�������
extern void IterativeCombos(int n,int m,int C_Arr[],int E_Arr[],int index[]); //��������Ͽ��� 
 

#endif  /*binarypuzzle.h*/ 
