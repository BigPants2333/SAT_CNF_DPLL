/*
 *  主程序，进行指定操作 
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
        printf("---------------基于DPLL的SAT求解器程序菜单-------------\n");
        printf("1. 求解CNF文件可满足性\n");
        printf("2. 二进制数独游戏\n");
        //printf("3. 帮助\n");
        printf("0. 退出\n");
        printf("-------------------------------------------------------\n");
        printf("请选择你的操作[0~2]: ");
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
            	printf("输入错误请重试！\n");
				getchar();
                getchar();
                break;
        }
    }
    printf("欢迎下次使用本系统！\n");
    getchar();
    getchar();
    return 0;
}
