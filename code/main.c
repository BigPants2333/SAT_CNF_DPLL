/*
 *  �����򣬽���ָ������ 
 */ 

#include "display.h" 

int main(int argc, char *argv[]) 
{
    int op=1;
    srand(time(NULL));
    while(op)
	{
        system("cls");
        printf("\n\n");
        printf("---------------����DPLL��SAT���������˵�-------------\n");
        printf("1. ���CNF�ļ���������\n");
        printf("2. ������������Ϸ\n");
        //printf("3. ����\n");
        printf("0. �˳�\n");
        printf("-------------------------------------------------------\n");
        printf("��ѡ����Ĳ���[0~2]: ");
        scanf("%d",&op);
        switch(op)
        {	
            case 1:
                CNFPage();
                getchar();
                getchar();
                break;
            case 2:
                binarypuzzlePage();
                getchar();
                getchar();
                break;
            /*
			case 3:
                helpPage();
                getchar();
                getchar();
                break;
            */
            case 0:
                break;
            default:
            	printf("������������ԣ�\n");
				getchar();
                getchar();
                break;
        }
    }
    printf("��ӭ�´�ʹ�ñ�ϵͳ��\n");
    getchar();
    getchar();
    return 0;
}
