#include "display.h"

void CNFPage(void)
{
    int dpll=0;
    int i,t;
    int op=1;
    int solvertype;
    int file_len;
    char filepath[256]="\0";
    char savepath[256];
    CNF_Solver *cnf=NULL;

    status firstflag=FALSE;
    long Handle;
    struct _finddata_t FileInfo;
    /*
	 *  Struct _finddata_t是用来存储文件各种信息的结构体，
	 *  使用这个结构体要引用的头文件为 #include <io.h> 
	 */

    while(op) 
	{
        system("cls");
        printf("\n\n");
        printf("---------------基于DPLL的SAT求解器程序菜单-------------\n");
        printf("---------------------------CNF-------------------------\n");
        printf("当前加载文件: %s\n",filepath);
        if(cnf) 
		{
            printf("文字数: %4d\n",cnf->literal_num);
            printf("子句数: %4d\n",cnf->clause_num);
            printf("-------------------------------------------------------\n");
        }
        printf("1. 加载CNF文件\n");
        printf("2. 求解CNF的可满足性\n");
        printf("3. 打印CNF\n");
        printf("4. 打印学习子句\n");
        printf("5. 检查答案\n");
        printf("0. 后退\n");
        printf("-------------------------------------------------------\n");
        printf("请选择你的操作[0~5]: ");
        scanf("%d",&op);
        switch(op) 
		{
        case 1:
            if(cnf) 
			{
                Destroy_Solver(cnf);
                cnf=NULL;
            }

            if(firstflag==TRUE) 
			{
                char ch;
                do 
				{
                    printf("是否选择下一个cnf文件? (Y/N)  ");
                    scanf("%*c%c", &ch);
                } while(ch!='Y' && ch!='y' && ch!='N' && ch!='n');
                if(ch=='N' || ch=='n') 
				{
                    firstflag = FALSE;
                    filepath[0] = '\0';
                }
            }
            if(firstflag==FALSE) 
			{
                printf("请输入CNF文件名: ");
                scanf("%s",filepath);
                file_len = strlen(filepath);

                if(strcmp(filepath+file_len-4, ".cnf")==0) 
				{
                    if(!(cnf=Load_CNF(filepath))) printf("文件打开失败！\n");
                    else printf("文件加载成功!\n");
                } 
				else if((Handle=_findfirst(strcat(filepath, "*.cnf"), &FileInfo))==-1L) 
				{
                    printf("没有找到匹配的文件！\n");
                    filepath[0] = '\0';
                } 
				else firstflag=TRUE;
            } 
			else
			{
                if(_findnext(Handle, &FileInfo)!=0) 
				{
                    printf("没有找到匹配的文件！\n");
                    _findclose(Handle);
                    firstflag = FALSE;
                    filepath[0] = '\0';
                }
            }

            if(firstflag==TRUE) 
			{
                filepath[file_len]='\0';
                printf("正在加载: %s\n",strcat(filepath, FileInfo.name));
                if(!(cnf=Load_CNF(filepath))) printf("文件打开失败！\n");
                else printf("文件加载成功!\n");
            }
            getchar();
            getchar();
            break;
        case 2:
            if(!cnf) printf("CNF不存在！\n");
            else 
			{
                t = -1;
                Clear_Solver_Answer(cnf);
                do 
				{
                    printf("请输入求解器(1.优化前求解器  2.优化后求解器): ");
                    scanf("%d", &solvertype);
                } while(solvertype != 1 && solvertype != 2);
                if(solvertype==1) 
				{
                    printf("\n正在使用\"优化前求解器\"求解...\n");
                    t=clock();
                    dpll=DPLL_Rec_1(cnf,0);
                    if(dpll==NOTSURE) dpll=FALSE;
                    else if(dpll==FALSE) dpll=NOTSURE;
                } 
				else 
				{
                    printf("\n正在使用\"优化后求解器\"求解...\n");
                    Create_Literal_Index(cnf);
                    t=clock();
                    dpll=DPLL_Rec_2(cnf,0);
                }
                t=clock()-t;
                if(dpll==TRUE) 
				{
                    printf("该CNF为可满足!\n");
                    printf("用时: %dms\n",t);
                    printf("求解答案如下:\n");
                    for(i=1; i<=cnf->literal_num; i++) 
					{
                        printf("%d, ",cnf->boolarray[i]*i);
                        if(i%10==0) printf("\n");
                    }
                    printf("\n");
                    To_Save_Path(savepath,filepath);
                    if(Save_Res(savepath,dpll,cnf,t)) printf("RES保存成功!\n");
                    else printf("RES保存失败!\n");
                }
				else
				{
                    printf("求解失败!\n");
                    printf("可能无解，可能在预定时间内无法解出，请查看RES文件\n");
                    printf("用时: %dms\n",t);
                    To_Save_Path(savepath,filepath);
                    if(Save_Res(savepath,dpll,cnf,t)) printf("RES保存成功!\n");
                    else printf("RES保存失败!\n");
                }
            }
            getchar();
            getchar();
            break;
        case 3:
            if(!cnf) printf("CNF不存在！\n");
			else 
			{
                printf("正在打印CNF...\n");
                Print_Clause(cnf);
                printf("打印完成！\n");
            }
            getchar();
            getchar();
            break;
        case 4:
            if(!cnf) printf("CNF不存在！\n");
            else 
			{
                printf("正在打印CNF...\n");
                Print_LearnClause(cnf);
                printf("打印完成！\n");
            }
            getchar();
            getchar();
            break;
        case 5:
            if(!cnf) printf("CNF不存在！\n");
            else if(dpll==0) printf("CNF求解出现错误!\n");
            else 
			{
                Check_Res(cnf);
                printf("打印完成！\n");
            }
            getchar();
            getchar();
            break;
        case 0:
            break;
        }
    }
    if(cnf) 
	{
        Destroy_Solver(cnf);
        cnf=NULL;
    }
}

void binarypuzzlePage(void)
{
	int dpll;
	int n,i;
	int op=1,choice=1,flag=1;
	BP_Solver *bp=NULL;
	char bp_path[256]="\0";
	char cnf_path[256]="\0";
	char *solved_bp=NULL,*dug_bp=NULL,*initial_bp=NULL;
	while(op)
	{
		system("cls");
        printf("\n\n");
        printf("---------------基于DPLL的SAT求解器程序菜单-------------\n");
        printf("-----------------------BinaryPuzzle--------------------\n");
        printf("1. 初始化二进制数独\n");
        printf("2. 求解你的二进制数独\n");
        printf("3. 求解随机二进制数独\n");
        printf("0. 后退\n");
        printf("请选择你的操作[0~3]: ");
		scanf("%d",&op);
        switch(op)
        {
        	case 1:
        		if(bp)
        		{
        			Destroy_Solver(bp);
					bp=NULL; 
				}
        		printf("请输入一个正整数n作为二进制数独阶数，n>=4且为偶数，\n");
        		printf("因该程序简单SAT求解器，建议输入4或6： ");
        		scanf("%d",&n);
        		bp=Create_BP_CNF(n);
        		if(bp) printf("二进制数独初始化成功！\n");
        		else printf("二进制数独初始化失败！\n"); 
				getchar();
				getchar();
        		break;
        	case 2:
        		if(bp)
        		{
        			Destroy_Solver(bp);
        			bp=NULL;
				}
				printf("请输入含有二进制数独字符串的文件名：\n");
				scanf("%s",bp_path);
				BP_file *file;
				file=Read_BP_File(bp_path);
				BP_node *p=file->head;
				choice=1;
				while(choice)
				{
					if(!p) break;
					system("cls");
                    printf("\n\n");
                    printf("---------------基于DPLL的SAT求解器程序菜单-------------\n");
                    printf("-----------------------BinaryPuzzle--------------------\n");
                    printf("求解给定的%d阶二进制数独，一共有%d个\n",file->n,file->num);
                    printf("正在处理第%d个\n",p->count);
                    bp=Create_BP_CNF(file->n);
                    printf("初始二进制数独如下：\n");
					Read_BP_Str(bp,file->n,p->s);
					Print_BP_Str(p->s,file->n);
                    printf("1. 保存CNF文件（请在进行操作2前使用该功能）\n");
                    printf("2. 使用DPLL进行求解\n");
                    printf("3. 求解下一个二进制数独\n");
                    printf("0. 后退\n");
                    printf("请选择你的操作[0~3]: ");
		            scanf("%d",&choice);
                    switch(choice)
					{
						case 1:
							printf("请输入保存路径\n");
							scanf("%s",cnf_path);
							if(Save_CNF(bp,cnf_path)) printf("保存CNF文件成功！\n");
							else printf("保存CNF文件失败！\n");
							getchar();
				            getchar();
        		            break;
						case 2:
							Create_Literal_Index(bp);
							dpll=DPLL_Rec_2(bp,0);
							if(dpll==TRUE)
							{
								printf("求解成功，答案如下\n");
								solved_bp=Solved_BP_Str(bp,file->n);
								Print_BP_Str(solved_bp,file->n);
							}
							else printf("出现错误！\n");
							getchar();
				            getchar();
        		            break;
						case 3:
							free(solved_bp);
							solved_bp=NULL;
							Destroy_Solver(bp);
							bp=NULL;
							p=p->next;
							getchar();
				            getchar();
        		            break;
        		        case 0:
        		        	free(solved_bp);
        		        	solved_bp=NULL;
        		        	Destroy_Solver(bp);
							bp=NULL;
						    break;
					} 
				}
        		getchar();
        		getchar();
        		break;
        	case 3:
        		if(!bp)
				{
					printf("请先初始化！\n");
					getchar();
					getchar();
					break; 
				}
				choice=1;
				while(choice)
				{
					system("cls");
                    printf("\n\n");
                    printf("---------------基于DPLL的SAT求解器程序菜单-------------\n");
                    printf("-----------------------BinaryPuzzle--------------------\n");
                    printf("初始化后，二进制数独为%d阶\n",n);
                    if(flag==0)
                    {
                    	//打印生成的完整棋盘 
					/*	if(initial_bp)
						{
							Print_BP_Str(initial_bp,n);
							for(i=0;i<n*2-1;i++) printf("-");
							printf("\n");
						}
                    */
					    //打印挖洞后的棋盘	
                    	if(dug_bp) 
						{
							Print_BP_Str(dug_bp,n);
							for(i=0;i<n*2-1;i++) printf("-");
							printf("\n");
						}
			            
			            if(choice==3) Print_BP_Str(solved_bp,n);
					} 
					
                    printf("1. 随机生成一个%d阶初始二进制数独\n",n);
                    printf("2. 保存CNF文件（请在进行操作3前使用该功能）\n");
                    printf("3. 使用DPLL进行求解\n");
                    printf("4. 生成下一个二进制数独\n");
                    printf("0. 后退\n");
                    printf("请选择你的操作[0~3]: ");
                    
                    if(flag==1)
					{
						do
						{
							printf("\n请先进行操作1或操作0\n");
							scanf("%d",&choice);
						}while(choice!=1 && choice!=0);
						
					}
                    else scanf("%d",&choice);
                    
		            switch(choice)
					{
						case 1:
							initial_bp=Initialize_BP(bp,n);
							//挖洞后的解与完整棋盘一致 
							while(1)
							{	
								dug_bp=Dig_Holes(initial_bp,n);
								bp=Create_Random_BP(bp,n,dug_bp);
								Create_Literal_Index(bp);
								dpll=DPLL_Rec_2(bp,0);
								solved_bp=Solved_BP_Str(bp,n);
								if(strcmp(initial_bp,solved_bp)==0) break;
								else
								{
									free(dug_bp);dug_bp=NULL;
									free(solved_bp);solved_bp=NULL;
								}
							}
						
							printf("随机生成初始二进制数独如下：\n");
							Print_BP_Str(dug_bp,n);
							flag=0;
							getchar();
				            getchar();
        		            break;
						case 2:
							printf("请输入保存路径\n");
							scanf("%s",cnf_path);
							if(Save_CNF(bp,cnf_path)) printf("保存CNF文件成功！\n");
							else printf("保存CNF文件失败！\n");
							flag=0;
							getchar();
				            getchar();
        		            break;
						case 3:
							bp=Create_Random_BP(bp,n,dug_bp);
							Create_Literal_Index(bp);
							dpll=DPLL_Rec_2(bp,0);
							if(dpll==TRUE)
							{
								printf("求解成功，答案如下\n");
								solved_bp=Solved_BP_Str(bp,n);
								Print_BP_Str(solved_bp,n);
							}
							else printf("出现错误！\n");
							flag=0;
							getchar();
				            getchar();
        		            break;
						case 4:
							free(initial_bp);free(dug_bp);free(solved_bp);
							initial_bp=NULL;dug_bp=NULL;solved_bp=NULL;
							Destroy_Solver(bp);bp=NULL;
							printf("该二进制数独已经销毁，确认后请进行操作1或操作0\n");
							bp=Create_BP_CNF(n);
							flag=0;
							getchar();
							getchar();
        		            break;
        		        case 0:
        		        	free(initial_bp);free(dug_bp);free(solved_bp);
							initial_bp=NULL;dug_bp=NULL;solved_bp=NULL;
							Destroy_Solver(bp);bp=NULL;
        		        	flag=1;
						    break;
					} 
				}
        		getchar();
        		getchar();
        		break;
        	case 0:
        		break;
		}
	}
	if(bp)
	{
		Destroy_Solver(bp);
		bp=NULL;
	}
}

void helpPage()
{
    system("cls");
    printf("\n\n");
    printf("---------------基于DPLL的SAT求解器程序菜单-------------\n");
    printf("--------------------------help-------------------------\n");
    printf("没有帮助的呢！\n");
    printf("-------------------------------------------------------\n");
    printf("按回车键继续...");
}
