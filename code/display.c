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
	 *  Struct _finddata_t�������洢�ļ�������Ϣ�Ľṹ�壬
	 *  ʹ������ṹ��Ҫ���õ�ͷ�ļ�Ϊ #include <io.h> 
	 */

    while(op) 
	{
        system("cls");
        printf("\n\n");
        printf("---------------����DPLL��SAT���������˵�-------------\n");
        printf("---------------------------CNF-------------------------\n");
        printf("��ǰ�����ļ�: %s\n",filepath);
        if(cnf) 
		{
            printf("������: %4d\n",cnf->literal_num);
            printf("�Ӿ���: %4d\n",cnf->clause_num);
            printf("-------------------------------------------------------\n");
        }
        printf("1. ����CNF�ļ�\n");
        printf("2. ���CNF�Ŀ�������\n");
        printf("3. ��ӡCNF\n");
        printf("4. ��ӡѧϰ�Ӿ�\n");
        printf("5. ����\n");
        printf("0. ����\n");
        printf("-------------------------------------------------------\n");
        printf("��ѡ����Ĳ���[0~5]: ");
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
                    printf("�Ƿ�ѡ����һ��cnf�ļ�? (Y/N)  ");
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
                printf("������CNF�ļ���: ");
                scanf("%s",filepath);
                file_len = strlen(filepath);

                if(strcmp(filepath+file_len-4, ".cnf")==0) 
				{
                    if(!(cnf=Load_CNF(filepath))) printf("�ļ���ʧ�ܣ�\n");
                    else printf("�ļ����سɹ�!\n");
                } 
				else if((Handle=_findfirst(strcat(filepath, "*.cnf"), &FileInfo))==-1L) 
				{
                    printf("û���ҵ�ƥ����ļ���\n");
                    filepath[0] = '\0';
                } 
				else firstflag=TRUE;
            } 
			else
			{
                if(_findnext(Handle, &FileInfo)!=0) 
				{
                    printf("û���ҵ�ƥ����ļ���\n");
                    _findclose(Handle);
                    firstflag = FALSE;
                    filepath[0] = '\0';
                }
            }

            if(firstflag==TRUE) 
			{
                filepath[file_len]='\0';
                printf("���ڼ���: %s\n",strcat(filepath, FileInfo.name));
                if(!(cnf=Load_CNF(filepath))) printf("�ļ���ʧ�ܣ�\n");
                else printf("�ļ����سɹ�!\n");
            }
            getchar();
            getchar();
            break;
        case 2:
            if(!cnf) printf("CNF�����ڣ�\n");
            else 
			{
                t = -1;
                Clear_Solver_Answer(cnf);
                do 
				{
                    printf("�����������(1.�Ż�ǰ�����  2.�Ż��������): ");
                    scanf("%d", &solvertype);
                } while(solvertype != 1 && solvertype != 2);
                if(solvertype==1) 
				{
                    printf("\n����ʹ��\"�Ż�ǰ�����\"���...\n");
                    t=clock();
                    dpll=DPLL_Rec_1(cnf,0);
                    if(dpll==NOTSURE) dpll=FALSE;
                    else if(dpll==FALSE) dpll=NOTSURE;
                } 
				else 
				{
                    printf("\n����ʹ��\"�Ż��������\"���...\n");
                    Create_Literal_Index(cnf);
                    t=clock();
                    dpll=DPLL_Rec_2(cnf,0);
                }
                t=clock()-t;
                if(dpll==TRUE) 
				{
                    printf("��CNFΪ������!\n");
                    printf("��ʱ: %dms\n",t);
                    printf("��������:\n");
                    for(i=1; i<=cnf->literal_num; i++) 
					{
                        printf("%d, ",cnf->boolarray[i]*i);
                        if(i%10==0) printf("\n");
                    }
                    printf("\n");
                    To_Save_Path(savepath,filepath);
                    if(Save_Res(savepath,dpll,cnf,t)) printf("RES����ɹ�!\n");
                    else printf("RES����ʧ��!\n");
                }
				else
				{
                    printf("���ʧ��!\n");
                    printf("�����޽⣬������Ԥ��ʱ�����޷��������鿴RES�ļ�\n");
                    printf("��ʱ: %dms\n",t);
                    To_Save_Path(savepath,filepath);
                    if(Save_Res(savepath,dpll,cnf,t)) printf("RES����ɹ�!\n");
                    else printf("RES����ʧ��!\n");
                }
            }
            getchar();
            getchar();
            break;
        case 3:
            if(!cnf) printf("CNF�����ڣ�\n");
			else 
			{
                printf("���ڴ�ӡCNF...\n");
                Print_Clause(cnf);
                printf("��ӡ��ɣ�\n");
            }
            getchar();
            getchar();
            break;
        case 4:
            if(!cnf) printf("CNF�����ڣ�\n");
            else 
			{
                printf("���ڴ�ӡCNF...\n");
                Print_LearnClause(cnf);
                printf("��ӡ��ɣ�\n");
            }
            getchar();
            getchar();
            break;
        case 5:
            if(!cnf) printf("CNF�����ڣ�\n");
            else if(dpll==0) printf("CNF�����ִ���!\n");
            else 
			{
                Check_Res(cnf);
                printf("��ӡ��ɣ�\n");
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
        printf("---------------����DPLL��SAT���������˵�-------------\n");
        printf("-----------------------BinaryPuzzle--------------------\n");
        printf("1. ��ʼ������������\n");
        printf("2. �����Ķ���������\n");
        printf("3. ����������������\n");
        printf("0. ����\n");
        printf("��ѡ����Ĳ���[0~3]: ");
		scanf("%d",&op);
        switch(op)
        {
        	case 1:
        		if(bp)
        		{
        			Destroy_Solver(bp);
					bp=NULL; 
				}
        		printf("������һ��������n��Ϊ����������������n>=4��Ϊż����\n");
        		printf("��ó����SAT���������������4��6�� ");
        		scanf("%d",&n);
        		bp=Create_BP_CNF(n);
        		if(bp) printf("������������ʼ���ɹ���\n");
        		else printf("������������ʼ��ʧ�ܣ�\n"); 
				getchar();
				getchar();
        		break;
        	case 2:
        		if(bp)
        		{
        			Destroy_Solver(bp);
        			bp=NULL;
				}
				printf("�����뺬�ж����������ַ������ļ�����\n");
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
                    printf("---------------����DPLL��SAT���������˵�-------------\n");
                    printf("-----------------------BinaryPuzzle--------------------\n");
                    printf("��������%d�׶�����������һ����%d��\n",file->n,file->num);
                    printf("���ڴ����%d��\n",p->count);
                    bp=Create_BP_CNF(file->n);
                    printf("��ʼ�������������£�\n");
					Read_BP_Str(bp,file->n,p->s);
					Print_BP_Str(p->s,file->n);
                    printf("1. ����CNF�ļ������ڽ��в���2ǰʹ�øù��ܣ�\n");
                    printf("2. ʹ��DPLL�������\n");
                    printf("3. �����һ������������\n");
                    printf("0. ����\n");
                    printf("��ѡ����Ĳ���[0~3]: ");
		            scanf("%d",&choice);
                    switch(choice)
					{
						case 1:
							printf("�����뱣��·��\n");
							scanf("%s",cnf_path);
							if(Save_CNF(bp,cnf_path)) printf("����CNF�ļ��ɹ���\n");
							else printf("����CNF�ļ�ʧ�ܣ�\n");
							getchar();
				            getchar();
        		            break;
						case 2:
							Create_Literal_Index(bp);
							dpll=DPLL_Rec_2(bp,0);
							if(dpll==TRUE)
							{
								printf("���ɹ���������\n");
								solved_bp=Solved_BP_Str(bp,file->n);
								Print_BP_Str(solved_bp,file->n);
							}
							else printf("���ִ���\n");
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
					printf("���ȳ�ʼ����\n");
					getchar();
					getchar();
					break; 
				}
				choice=1;
				while(choice)
				{
					system("cls");
                    printf("\n\n");
                    printf("---------------����DPLL��SAT���������˵�-------------\n");
                    printf("-----------------------BinaryPuzzle--------------------\n");
                    printf("��ʼ���󣬶���������Ϊ%d��\n",n);
                    if(flag==0)
                    {
                    	//��ӡ���ɵ��������� 
					/*	if(initial_bp)
						{
							Print_BP_Str(initial_bp,n);
							for(i=0;i<n*2-1;i++) printf("-");
							printf("\n");
						}
                    */
					    //��ӡ�ڶ��������	
                    	if(dug_bp) 
						{
							Print_BP_Str(dug_bp,n);
							for(i=0;i<n*2-1;i++) printf("-");
							printf("\n");
						}
			            
			            if(choice==3) Print_BP_Str(solved_bp,n);
					} 
					
                    printf("1. �������һ��%d�׳�ʼ����������\n",n);
                    printf("2. ����CNF�ļ������ڽ��в���3ǰʹ�øù��ܣ�\n");
                    printf("3. ʹ��DPLL�������\n");
                    printf("4. ������һ������������\n");
                    printf("0. ����\n");
                    printf("��ѡ����Ĳ���[0~3]: ");
                    
                    if(flag==1)
					{
						do
						{
							printf("\n���Ƚ��в���1�����0\n");
							scanf("%d",&choice);
						}while(choice!=1 && choice!=0);
						
					}
                    else scanf("%d",&choice);
                    
		            switch(choice)
					{
						case 1:
							initial_bp=Initialize_BP(bp,n);
							//�ڶ���Ľ�����������һ�� 
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
						
							printf("������ɳ�ʼ�������������£�\n");
							Print_BP_Str(dug_bp,n);
							flag=0;
							getchar();
				            getchar();
        		            break;
						case 2:
							printf("�����뱣��·��\n");
							scanf("%s",cnf_path);
							if(Save_CNF(bp,cnf_path)) printf("����CNF�ļ��ɹ���\n");
							else printf("����CNF�ļ�ʧ�ܣ�\n");
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
								printf("���ɹ���������\n");
								solved_bp=Solved_BP_Str(bp,n);
								Print_BP_Str(solved_bp,n);
							}
							else printf("���ִ���\n");
							flag=0;
							getchar();
				            getchar();
        		            break;
						case 4:
							free(initial_bp);free(dug_bp);free(solved_bp);
							initial_bp=NULL;dug_bp=NULL;solved_bp=NULL;
							Destroy_Solver(bp);bp=NULL;
							printf("�ö����������Ѿ����٣�ȷ�Ϻ�����в���1�����0\n");
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
    printf("---------------����DPLL��SAT���������˵�-------------\n");
    printf("--------------------------help-------------------------\n");
    printf("û�а������أ�\n");
    printf("-------------------------------------------------------\n");
    printf("���س�������...");
}
