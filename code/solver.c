#include "solver.h"
#include<time.h>

#define VSIDS 2
#define VSIDSCOUNT 100
#define LEARNLENGTH_MAX 20
#define TIMEOUT 720000

//===========================================================================================
//全局变量 

int startTime;
status restart=FALSE;
int MAXC = 10;  //回溯次数上限，超过就重启动 

//===========================================================================================
//策略相关函数 

int MOM_Strategy(CNF_Solver *cnf)
{
    int i;
    double max=-1;
    int index=0;
    Clause *Cp;
    Literal *Lp;
    double *J=(double*)malloc(sizeof(double)*(cnf->literal_num+1));
    for(i=1; i<=cnf->literal_num; i++) J[i]=0;
    for(Cp=cnf->root; Cp; Cp=Cp->next) 
	{
        for(Lp=Cp->head; Lp; Lp=Lp->next) 
            J[ABS(Lp->literal)]+=1.0/(1<<(Cp->length));
    }
    for(i=1; i<=cnf->literal_num; i++) 
	{
        if(max<J[i] && cnf->boolarray[i]==NOTSURE) 
		{
            max=J[i];
            index = i;
        }
    }
    int pos=0,neg=0;
    for(Cp=cnf->root; Cp; Cp=Cp->next) 
	{
        for(Lp=Cp->head; Lp; Lp=Lp->next) 
		{
            if(Lp->literal>0) pos++;
            else neg++;
        }
    }
    if(pos<neg) index=-index;
    free(J);
    return index;
}

int VSIDS_Strategy(CNF_Solver *cnf)
{
    int i;
    static int count=0;
    int max=-1;
    int index=0;
    for(i=1; i<=cnf->literal_num; i++) 
	{
        if(max<cnf->countarray[i]+cnf->countarray[i+cnf->literal_num] 
		        && cnf->boolarray[i]==NOTSURE) 
		{
            max=cnf->countarray[i]+cnf->countarray[i+cnf->literal_num];
            index = i;
        }
    }
    count++;
    if(count>VSIDSCOUNT) 
	{
        count=0;
        for(i=1; i<=cnf->literal_num*2; i++) cnf->countarray[i]/=VSIDS;
    }
    index=(cnf->countarray[index]>cnf->countarray[index+cnf->literal_num])?index:-index;
    return index;
}

int Frequency_Strategy(CNF_Solver *cnf)
{
    int i;
    int max=0;
    int index=0;
    Clause *Cp;
    Literal *Lp;
    int *visitedplus=(int*)malloc((cnf->literal_num+1)*sizeof(int));
    int *visitedminus=(int*)malloc((cnf->literal_num+1)*sizeof(int));
    for(i=1; i<=cnf->literal_num; i++) visitedplus[i]=visitedminus[i]=0;
    for(Cp=cnf->root; Cp; Cp=Cp->next) 
	{
        for(Lp=Cp->head; Lp; Lp=Lp->next) 
		{
            if(Lp->literal>0) visitedplus[Lp->literal]++;
            else visitedminus[0-Lp->literal]++;
        }
    }
    for(i=1; i<=cnf->literal_num; i++) 
	{
        if(max<visitedplus[i]+visitedminus[i]) 
		{
            max=visitedplus[i]+visitedminus[i];
            index = i;
        }
    }
    index=(visitedplus[index]<visitedminus[index])?-index:index;
    free(visitedplus);
    free(visitedminus);
    return index;
}

int Weight_Strategy(CNF_Solver *cnf)
{
    int i;
    int index=0;
    Clause *Cp;
    int w=0;

    Cp=cnf->root;
    int ran = rand()%(cnf->clause_num);
    for(i=1; i<ran; i++) {
        Cp=Cp->next;
    }
    Cp->weight++;

    i=0;
    while(w==0 && i<=cnf->literal_num) 
	{
        i++;
        for(; i<=cnf->literal_num && cnf->boolarray[i]!=NOTSURE; i++);
        for(Cp=cnf->root; Cp; Cp=Cp->next) 
		{
            if(Evaluate_Clause(Cp, i)!=TRUE) w-=Cp->weight;
            if(Evaluate_Clause(Cp, -i)!=TRUE) w+=Cp->weight;
        }
        if(w>0) index = i;
        else if(w<0) index = -i;
    }
    return index;
}

int Combine_Strategy(CNF_Solver *cnf, enum strategy s)
{
    int index=0;
    switch(s) 
	{
        case mom:
            index = MOM_Strategy(cnf);
            break;
        case vsids:
            index = VSIDS_Strategy(cnf);
            break;
        case first:
            index = cnf->root->head->literal;
            break;
        case frequency:
            index = Frequency_Strategy(cnf);
            break;
        case weight_s:
            index = Weight_Strategy(cnf);
            break;
    }
    return index;
}

//===========================================================================================
//学习子句相关函数

status Recover_Change(CNF_Solver *cnf, Stack *head, int time, int *learnarray)
{
/* 
 *  learnarray组成:第0个元素为长度，第1个开始是文字,第i+1个为唯一蕴含点
 *  学习子句计划
 * 
 *  加入a[]:文字x在当前层的赋值状态X=boolarray[x]*x加入a[]
 * 
 *  第一次撤销肯定是冲突的子句C变空，所以第一次撤销后该子句C的DIVIDE后的所有文字都加入a[]
 *
 *  1.将冲突的文字加入a[]_0
 *
 *  2.第i次撤销后，先检查该子句是否有元素，若该子句的元素只有一个(tag为CLAUSE)，
 *    且元素在a[]_0内（元素的值取反后与a[]_0比较），
 *    该子句C的DIVIDE后的所有文字都加入a[]_0
 *    “若该子句的元素只有一个”：是不是蕴含关系
 *    “且元素在a[]_0内”：是否与冲突子句有关
 *
 *  3.将所有撤销完成后，得到a[]_1，删去a[]_1中本层所有的变量赋值
 *   （决策以及推倒出）的文字，得到a[]_2
 *    本层：a[]_1中文字i的stagearray[i]的最大值F
 *
 *  4.得到需要返回的决策层，即a[]_2中文字i的stagearray[i]的最大值toF
 *
 *  5.加入本层的决策X得到a[]
 *  
 *  决策X：a[]_1中文字i的stagearray[i]的值为F且为最后一个(由后往前遍历)
 *
 *  6.插入学习子句Cl => a[]得到最后蕴含关系
 */
    int i=1,j;
    int toF=head->floor;
    status *G=NULL;
    status *Gvisited=NULL;
    Literal *Lp;
    Stack *Sp=head->next;
    status flag=FALSE;
    status isLearn=FALSE;
    while(Sp && time!=0) 
	{
        //Print_Stack(head);
        switch(Sp->tag) 
		{
            case CLAUSE:
                Insert_Clause_Top(cnf,Sp->Cp);
                break;
            case LITERAL:
                //子句文字遵循规律：DIVIDE后的文字决策层为降序
                //撤销本层的文字删除操作只需要将本决策层的文字放在DIVIDE前即可
                Recover_Literal(Sp->Cp,Sp->Lp);
                if(learnarray && flag==FALSE) 
				{
                    //将冲突的文字加入a[]_0
                    if(Sp->Cp->length==1) 
					{
                        flag=TRUE;
                        G=(status*)malloc(sizeof(status)*(cnf->literal_num+2));
                        Gvisited = (status*)malloc(sizeof(status)*(cnf->literal_num+1));
                        for(j=1; j<=cnf->literal_num; j++) Gvisited[j]=FALSE;
                        learnarray[i]=Sp->Cp->head->literal;
                        i++;
                        learnarray[i]=0-Sp->Cp->head->literal;
                        i++;
                        Gvisited[ABS(Sp->Cp->head->literal)]=TRUE;
                    }
                }
                break;
            case SPLIT:
                Delete_Clause(cnf, Sp->Cp);
                break;
            default:
                break;
        }

        if(learnarray && isLearn==FALSE) 
		{
            if(Sp->Cp->length==1) 
			{
                //是否元素在a[]_0内（元素的值取反后与a[]_0比较）
                //监视哨learnarray[i]=0-Sp->Cp->head->literal
                learnarray[i]=0-Sp->Cp->head->literal;
                for(j=1; Sp->Cp->head->literal!=-learnarray[j]; j++) ;
                if(j!=i) 
				{
                    //访问该顶点
                    G[j]=TRUE;
                    int t=j;
                    //是否全部访问过
                    for(j=2; j<i; j++) 
					{
                        if(G[j]!=TRUE) break;
                    }
                    if(j==i && i!=3 && isLearn==FALSE) 
					{
                        isLearn=TRUE;

                        learnarray[0]=i-1;
                        learnarray[i]=-learnarray[t];
                        int f=Create_LearnClause(cnf,learnarray+1,learnarray[0],-learnarray[t]);
                        if(f<toF) toF=f;
                    } 
					else 
					{
                        for(Lp=Sp->Cp->rmv; Lp; Lp=Lp->next) 
						{
                            //删去重复的文字 
                            if(Gvisited[ABS(Lp->literal)]==FALSE) 
							{
                                learnarray[i]=Lp->literal;
                                G[i]=FALSE;
                                Gvisited[ABS(Lp->literal)] = TRUE;
                                if(cnf->floorarray[ABS(learnarray[i])]<head->floor)
                                    G[i]=TRUE;
                                i++;
                            }
                        }
                    }
                }
            }
        }
        time--;
        head->next=head->next->next;
        free(Sp);
        Sp=head->next;
    }
    if(learnarray) 
	{
        free(G);
        free(Gvisited);
        learnarray[0]=toF;
    }
    return OK;
}

int Create_LearnClause(CNF_Solver *cnf, int *a, int i, int X)
{
    int j,l;
    int *copy = a;
    int F=0,toF=0;
    //由后往前减少赋值次数
    for(j=i-1; j>=0; j--) 
	{
        if(F<cnf->floorarray[ABS(copy[j])]) F=cnf->floorarray[ABS(copy[j])];
    }
    //将所有撤销完成后，得到a[]_1，
    //删去a[]_1中本层所有的变量赋值（决策以及推倒出）的文字，得到a[]_2
    for(l=0,j=0; j<i; j++) {
        if(cnf->floorarray[ABS(copy[j])]!=F) 
		{
            copy[l]=copy[j];
            l++;
        }
    }
    //得到需要返回的决策层，即a[]_2中文字i的stagearray[i]的最大值toF
    for(j=0; j<l; j++) {
        if(toF<cnf->floorarray[ABS(copy[j])]) toF=cnf->floorarray[ABS(copy[j])];
    }
    //如果学习子句长度过长，只生成决策单子句，在回溯的时候将其删除
	int L=l+1;  //长学习子句的原长

    //排序变元，使得变元决策层为降序
    Floor *FF = (Floor*)malloc(sizeof(Floor)*l);
    for(i=0; i<l; i++) 
	{
        FF[i].a = copy[i];
        FF[i].floor = cnf->floorarray[ABS(copy[i])];
    }
    qsort(FF, l, sizeof(Floor), Compare_Des);  //调用内置快排
    for(i=0; i<l; i++) copy[i] = FF[i].a;
    free(FF);

    //加入本层的决策X得到a[]
    copy[l]=-X;
    l++;
    int tmp;
    for(j=l-1; j>0; j--) 
	{
        tmp=copy[j-1];
        copy[j-1]=copy[j];
        copy[j]=tmp;
    }

    //插入学习子句Cl => a[]得到最后蕴含关系
    Add_Clause(cnf,l,copy);

    LearnClause *lcp=(LearnClause*)malloc(sizeof(LearnClause));
    lcp->isInStack=FALSE;
    lcp->floor=F;
    lcp->clause=cnf->root;
    lcp->call_num=L;
    lcp->next=cnf->learn_root;
    cnf->learn_root=lcp;

    //VSIDS
    for(i=0; i<L; i++) {
        if(copy[i]>0) cnf->countarray[copy[i]]++;
        else cnf->countarray[-copy[i]+cnf->literal_num]++;
        
    }

    //添加学习子句的索引
    Add_Literal_Index(cnf,lcp->clause);

    //学习子句的文字有赋值
    while(cnf->root->head) 
	{
        Remove_Literal(cnf->root,cnf->root->head->literal);
    }
    return toF;
}

status Backtrack_LearnClause(CNF_Solver *cnf, int floor)
{
    Literal *Lp;
    LearnClause *lcp=cnf->learn_root;
    while(lcp) 
	{
        if(lcp->isInStack==FALSE) 
		{
            for(Lp=lcp->clause->rmv; Lp && cnf->floorarray[ABS(Lp->literal)]==floor; 
			    Lp=lcp->clause->rmv) Recover_Literal(lcp->clause,Lp);
            if(Lp==NULL) lcp->isInStack=TRUE;
        }
        lcp=lcp->next;
    }
    return OK;
}

status Delete_Repeat_LearnClause(CNF_Solver *cnf, LearnClause *Lcp)
{
    Literal *Lp1,*Lp2;
    LearnClause *lcp=cnf->learn_root;

    lcp=Lcp->next;
    if(lcp) 
	{
        Lp1=lcp->clause->head;
        if(Lp1 && Lp1->literal==Lcp->clause->head->literal) 
		{
            for(Lp1=lcp->clause->rmv,Lp2=cnf->learn_root->clause->rmv; Lp1 && Lp2; 
			    Lp1=Lp1->next,Lp2=Lp2->next) 
			{
                if(Lp1->literal!=Lp2->literal) break;
            }
            if(Lp1==NULL && Lp2==NULL) 
			{
                LearnClause *tmp=lcp->next;
                Delete_LearnClause(cnf,lcp);
                lcp=tmp;
            } else lcp=lcp->next;
        }
    }
    return OK;
}

//===========================================================================================
//子句化简相关函数

status Simplify_UnitClause_1(CNF_Solver *cnf, int literal, Stack *head)
{
    Literal *Lp;
    Clause *Cp=cnf->root;
    Clause *Cq;
    while(Cp) 
	{
        switch(Evaluate_Clause(Cp,literal)) 
		{
            case TRUE:
                Remove_Clause(cnf, Cp);
                if(head) Save_Change(head,CLAUSE,Cp,NULL);
                Cp=Cp->next;
                //Print_Clause(cnf);
                break;
            case NOTSURE:
                Lp=Remove_Literal(Cp,-literal);
                if(head) Save_Change(head,LITERAL,Cp,Lp);
                //如果为空语句,把空语句置顶返回
                //如果为单语句,置顶
                if(Cp->length<=1) 
	    		{
                    Cq=Cp->next;
                    Remove_Clause(cnf,Cp);
                    Insert_Clause_Top(cnf,Cp);
                    if(Cp->length==0) return ERROR;
                    else Cp=Cq;   
                } 
		    	else Cp=Cp->next;
                //Print_Clause(cnf);
                break;
            default:
                Cp=Cp->next;
                break;
        }
    }
    return OK;
}

status Simplify_UnitClause_2(CNF_Solver *cnf, Clause *Cp, Stack *head)
{
    int literal = Cp->head->literal;
    int index = ABS(literal);

    LiteralIndex *li;
    Clause *CCp;
    Literal *Lp;
    //由于插入的单子句没有加入索引，必须另外删除
    Remove_Clause(cnf, Cp);
    Save_Change(head,CLAUSE,Cp,NULL);
    for(li=cnf->lindex[index].next; li; li=li->next) 
	{
        CCp=li->Cp;
        Lp=li->Lp;
        if(CCp->isremoved==FALSE) 
		{
            if(Lp->literal == literal) 
			{
                Remove_Clause(cnf, CCp);
                if(head) Save_Change(head,CLAUSE,CCp,NULL);
            } 
			else if(Lp->literal == -literal) 
			{
                Lp = Remove_Literal(CCp,-literal);
                if(head) Save_Change(head,LITERAL,CCp,Lp);
                //如果为空语句,把空语句置顶返回
                //如果为单语句,置顶
                if(CCp->length<=1) 
				{
                    Remove_Clause(cnf,CCp);
                    Insert_Clause_Top(cnf,CCp);
                    if(Cp->length==0) return ERROR;
                }
            }
        }
    }
    return OK;
}

//===========================================================================================
//辅助函数

status Save_Change(Stack *head, int tag, Clause* Cp, Literal *Lp)
{
    //head是不含数据的头节点指针
    if(head==NULL) return FALSE;
    Stack *newChange = (Stack*)malloc(sizeof(Stack));
    if(!newChange) return ERROR;
    newChange->tag = tag;
    switch(tag) 
	{
        case CLAUSE:
            newChange->Cp=Cp;  //保存子句
            break;
        case LITERAL:
            newChange->Cp=Cp;  //保存所在子句地址
            newChange->Lp=Lp;  //保存变元
            break;
        case SPLIT:
            newChange->Cp=Cp;  //保存子句
            break;
        default:
            break;
    }
    newChange->next=head->next;
    head->next=newChange;
    return OK;
}

status Backtrack_Assign(CNF_Solver *cnf, int floor)
{
    int j;
    for(j=1; j<=cnf->literal_num; j++) 
	{
        if(cnf->floorarray[j]==floor) 
		{
            cnf->boolarray[j]=NOTSURE;
            cnf->floorarray[j]=-1;
        }
    }
    return OK;
}

status Check_Res(CNF_Solver *cnf)
{
    int i,b;
    int flag;
    Clause *Cp;
    Literal *Lp;
    printf("-------------------------------------------------------\n");
    printf("正在检查: \n");
    for(Cp=cnf->root,i=1; Cp; Cp=Cp->next,i++) 
	{
        printf("第%d个子句: ",i);
        for(Lp=Cp->head,flag=0; Lp; Lp=Lp->next)
		{
            b=cnf->boolarray[(ABS(Lp->literal))];
            if(ASSIGN(Lp->literal)==b) 
			{
                printf("√");
                flag=1;
                break;
            } 
			else if(b==NOTSURE) printf("%dNOTSURE ",ABS(Lp->literal));
            else if(ASSIGN(Lp->literal)==-b) continue;
        }
        if(!flag) printf("x");
        if(i%4==0) printf("\n");
        else printf("\t\t");
    }
    printf("\n");
    return OK;
}

status Print_Stack(Stack *head)
{
    Stack *p;
    printf("-------------------------------------------------------\n");
    printf("打印栈\n");
    for(p=head->next; p; p=p->next) 
	{
        switch(p->tag) 
		{
            case CLAUSE:
                printf("CLAUSE:\n");
                printf("loc:%p\n",p->Cp);
                break;
            case LITERAL:
                printf("LITERAL:\n");
                printf("Literal:%p Clause:%p\n",p->Lp,p->Cp);
                break;
            case SPLIT:
                printf("SPLIT:\n");
                printf("split:%p\n",p->Cp);
                break;
            default:
                break;
        }
    }
    return OK;
}

int Compare_Des(const void *a,const void *b)
{
    return ((Floor*)b)->floor - ((Floor*)a)->floor;
}

//===========================================================================================
//DPLL递归求解函数

boolean DPLL_Rec_1(CNF_Solver *cnf, const int f)
{
    int l;
    Clause *Cp;
    Stack st;
    st.next=NULL;
    if(f==0) {
        startTime=clock();
    }
    while((Cp=Find_Unit_Clause(cnf))!=NULL)  //单子句传播
	{
        Boolarray_Assign(cnf,Cp->head->literal,f);  //记录到数组
        Simplify_UnitClause_1(cnf, Cp->head->literal, &st);  //化简单子句
        if(Have_Empty_Clause(cnf)==TRUE)  //如果有空子句
		{
            Recover_Change(cnf,&st,-1,NULL);
            Backtrack_Assign(cnf,f);
            if(clock()-startTime>=TIMEOUT) 
			{
                printf("超时 >%dms\n",TIMEOUT);
                return NOTSURE;
            }
            return FALSE;
        } else if(Have_Clause(cnf)==FALSE)  //如果没有子句
		{
            Recover_Change(cnf,&st,-1,NULL);
            return TRUE;
        }
    }//while
    l=Combine_Strategy(cnf,mom);  //综合策略选出变元
    Boolarray_Assign(cnf,l,f);  //记录到数组
    Add_Clause(cnf,1,&l);  //添加到子句集中
    Save_Change(&st,SPLIT,cnf->root,NULL);
    int s=DPLL_Rec_1(cnf, f+1);  //递归
    if(s==TRUE)  //如果是真
	{ 
        Recover_Change(cnf,&st,-1,NULL);
        return TRUE;
    }
    if(s==NOTSURE) 
	{
        Recover_Change(cnf,&st,-1,NULL);
        Backtrack_Assign(cnf,f);
        return NOTSURE;
    }
    Recover_Change(cnf,&st,1,NULL);
    l=-l;  //取假值
    Boolarray_Assign(cnf,l,f);  //记录到数组
    Add_Clause(cnf,1,&l);  //添加到子句集中
    Save_Change(&st,SPLIT,cnf->root,NULL);
    s=DPLL_Rec_1(cnf, f+1);  //递归
    Recover_Change(cnf,&st,-1,NULL);
    if(s==FALSE) Backtrack_Assign(cnf,f);
    return s;
}

boolean DPLL_Rec_2(CNF_Solver *cnf,const int f)
{
    int s,l;
    int count=0;
    int ret=f;  //回溯层数
    Clause *Cp;
    Stack st;
    st.floor=f;
    st.next=NULL;
    //VSIDS对每个文字设立一个计数器
	//当文字在某个子句中出现，包括原始问题中的子句和学习子句，该文字对应的计数器就加1
    if(f==0) 
	{
        startTime = clock();
        Literal *Lp;
        for(Cp=cnf->root; Cp; Cp=Cp->next) 
		{
            for(Lp=Cp->head; Lp; Lp=Lp->next) 
			{
                if(Lp->literal>0) cnf->countarray[Lp->literal]++;
                else cnf->countarray[0-Lp->literal+cnf->literal_num]+=1;
            }
        }
    }
    do 
	{
        count++;
        if(restart==TRUE) 
		{
            restart=FALSE;
            //如果重启动，可以把那些没有移除文字的、很长的学习子句删除
            LearnClause *lcp=cnf->learn_root,*lcq=NULL;
            while(lcp) 
			{
                if(lcp->clause->rmv==NULL && lcp->clause->length>LEARNLENGTH_MAX) 
				{
                    lcq=lcp->next;
                    Delete_Clause_Literal_Index(cnf, lcp->clause);
                    Delete_LearnClause(cnf,lcp);
                    lcp=lcq;
                } 
				else lcp=lcp->next;
            }
        }
        while((Cp=Find_Unit_Clause(cnf))!=NULL)  //单子句传播
		{
            Boolarray_Assign(cnf,Cp->head->literal,f);  //记录到数组
            Simplify_UnitClause_2(cnf, Cp, &st);  //化简单子句并保存修改
            if(Have_Empty_Clause(cnf)==TRUE)  //如果有空子句
			{
                if(clock()-startTime>=TIMEOUT) 
				{
                    printf("超时 >%dms\n",TIMEOUT);
                    return -1-f;
                }
                int *a=(int*)malloc(sizeof(int)*(cnf->literal_num+3));
                Recover_Change(cnf, &st,-1, a);  //还原到初始集
                //如果取值不成立，可能要添加学习子句并非时序回溯
                ret=a[0];
                free(a);
                Backtrack_LearnClause(cnf,f);
                Backtrack_Assign(cnf,f);  //赋值回溯
                if(count>MAXC) 
				{
                    MAXC++;
                    //printf("重启动...\n");
                    restart=TRUE;
                    //学习子句的回溯
                    return 0-f;
                }
                return ret-f;
            } 
			else if(Have_Clause(cnf)==FALSE)  //如果没有子句
			{
                Recover_Change(cnf, &st,-1, NULL);//还原到初始集
                //学习子句的回溯
                Backtrack_LearnClause(cnf, f);
                return TRUE;
            }
        }//while
        
        l=Combine_Strategy(cnf,mom);  //综合策略选出变元

        Add_Clause(cnf,1,&l);
        Save_Change(&st,SPLIT,cnf->root,NULL);  //保存插入操作

        //printf("递归:%d, 选%d\n",f,l);

        s=DPLL_Rec_2(cnf,f+1);
        Recover_Change(cnf,&st,1,NULL);
    } while(s==FALSE);  //do-while
    if(s==TRUE) 
	{
        Recover_Change(cnf,&st,-1,NULL);
        Backtrack_LearnClause(cnf,f);  //学习子句的回溯
        return TRUE;
    }
    else  //按返回值多层回溯
	{
        Recover_Change(cnf,&st,-1,NULL);
        Backtrack_LearnClause(cnf,f);  //学习子句的回溯
        Backtrack_Assign(cnf,f);  //赋值回溯
        return s+1;
    }
}
