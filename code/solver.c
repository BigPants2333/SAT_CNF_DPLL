#include "solver.h"
#include<time.h>

#define VSIDS 2
#define VSIDSCOUNT 100
#define LEARNLENGTH_MAX 20
#define TIMEOUT 720000

//===========================================================================================
//ȫ�ֱ��� 

int startTime;
status restart=FALSE;
int MAXC = 10;  //���ݴ������ޣ������������� 

//===========================================================================================
//������غ��� 

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
//ѧϰ�Ӿ���غ���

status Recover_Change(CNF_Solver *cnf, Stack *head, int time, int *learnarray)
{
/* 
 *  learnarray���:��0��Ԫ��Ϊ���ȣ���1����ʼ������,��i+1��ΪΨһ�̺���
 *  ѧϰ�Ӿ�ƻ�
 * 
 *  ����a[]:����x�ڵ�ǰ��ĸ�ֵ״̬X=boolarray[x]*x����a[]
 * 
 *  ��һ�γ����϶��ǳ�ͻ���Ӿ�C��գ����Ե�һ�γ�������Ӿ�C��DIVIDE����������ֶ�����a[]
 *
 *  1.����ͻ�����ּ���a[]_0
 *
 *  2.��i�γ������ȼ����Ӿ��Ƿ���Ԫ�أ������Ӿ��Ԫ��ֻ��һ��(tagΪCLAUSE)��
 *    ��Ԫ����a[]_0�ڣ�Ԫ�ص�ֵȡ������a[]_0�Ƚϣ���
 *    ���Ӿ�C��DIVIDE����������ֶ�����a[]_0
 *    �������Ӿ��Ԫ��ֻ��һ�������ǲ����̺���ϵ
 *    ����Ԫ����a[]_0�ڡ����Ƿ����ͻ�Ӿ��й�
 *
 *  3.�����г�����ɺ󣬵õ�a[]_1��ɾȥa[]_1�б������еı�����ֵ
 *   �������Լ��Ƶ����������֣��õ�a[]_2
 *    ���㣺a[]_1������i��stagearray[i]�����ֵF
 *
 *  4.�õ���Ҫ���صľ��߲㣬��a[]_2������i��stagearray[i]�����ֵtoF
 *
 *  5.���뱾��ľ���X�õ�a[]
 *  
 *  ����X��a[]_1������i��stagearray[i]��ֵΪF��Ϊ���һ��(�ɺ���ǰ����)
 *
 *  6.����ѧϰ�Ӿ�Cl => a[]�õ�����̺���ϵ
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
                //�Ӿ�������ѭ���ɣ�DIVIDE������־��߲�Ϊ����
                //�������������ɾ������ֻ��Ҫ�������߲�����ַ���DIVIDEǰ����
                Recover_Literal(Sp->Cp,Sp->Lp);
                if(learnarray && flag==FALSE) 
				{
                    //����ͻ�����ּ���a[]_0
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
                //�Ƿ�Ԫ����a[]_0�ڣ�Ԫ�ص�ֵȡ������a[]_0�Ƚϣ�
                //������learnarray[i]=0-Sp->Cp->head->literal
                learnarray[i]=0-Sp->Cp->head->literal;
                for(j=1; Sp->Cp->head->literal!=-learnarray[j]; j++) ;
                if(j!=i) 
				{
                    //���ʸö���
                    G[j]=TRUE;
                    int t=j;
                    //�Ƿ�ȫ�����ʹ�
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
                            //ɾȥ�ظ������� 
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
    //�ɺ���ǰ���ٸ�ֵ����
    for(j=i-1; j>=0; j--) 
	{
        if(F<cnf->floorarray[ABS(copy[j])]) F=cnf->floorarray[ABS(copy[j])];
    }
    //�����г�����ɺ󣬵õ�a[]_1��
    //ɾȥa[]_1�б������еı�����ֵ�������Լ��Ƶ����������֣��õ�a[]_2
    for(l=0,j=0; j<i; j++) {
        if(cnf->floorarray[ABS(copy[j])]!=F) 
		{
            copy[l]=copy[j];
            l++;
        }
    }
    //�õ���Ҫ���صľ��߲㣬��a[]_2������i��stagearray[i]�����ֵtoF
    for(j=0; j<l; j++) {
        if(toF<cnf->floorarray[ABS(copy[j])]) toF=cnf->floorarray[ABS(copy[j])];
    }
    //���ѧϰ�Ӿ䳤�ȹ�����ֻ���ɾ��ߵ��Ӿ䣬�ڻ��ݵ�ʱ����ɾ��
	int L=l+1;  //��ѧϰ�Ӿ��ԭ��

    //�����Ԫ��ʹ�ñ�Ԫ���߲�Ϊ����
    Floor *FF = (Floor*)malloc(sizeof(Floor)*l);
    for(i=0; i<l; i++) 
	{
        FF[i].a = copy[i];
        FF[i].floor = cnf->floorarray[ABS(copy[i])];
    }
    qsort(FF, l, sizeof(Floor), Compare_Des);  //�������ÿ���
    for(i=0; i<l; i++) copy[i] = FF[i].a;
    free(FF);

    //���뱾��ľ���X�õ�a[]
    copy[l]=-X;
    l++;
    int tmp;
    for(j=l-1; j>0; j--) 
	{
        tmp=copy[j-1];
        copy[j-1]=copy[j];
        copy[j]=tmp;
    }

    //����ѧϰ�Ӿ�Cl => a[]�õ�����̺���ϵ
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

    //���ѧϰ�Ӿ������
    Add_Literal_Index(cnf,lcp->clause);

    //ѧϰ�Ӿ�������и�ֵ
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
//�Ӿ仯����غ���

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
                //���Ϊ�����,�ѿ�����ö�����
                //���Ϊ�����,�ö�
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
    //���ڲ���ĵ��Ӿ�û�м�����������������ɾ��
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
                //���Ϊ�����,�ѿ�����ö�����
                //���Ϊ�����,�ö�
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
//��������

status Save_Change(Stack *head, int tag, Clause* Cp, Literal *Lp)
{
    //head�ǲ������ݵ�ͷ�ڵ�ָ��
    if(head==NULL) return FALSE;
    Stack *newChange = (Stack*)malloc(sizeof(Stack));
    if(!newChange) return ERROR;
    newChange->tag = tag;
    switch(tag) 
	{
        case CLAUSE:
            newChange->Cp=Cp;  //�����Ӿ�
            break;
        case LITERAL:
            newChange->Cp=Cp;  //���������Ӿ��ַ
            newChange->Lp=Lp;  //�����Ԫ
            break;
        case SPLIT:
            newChange->Cp=Cp;  //�����Ӿ�
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
    printf("���ڼ��: \n");
    for(Cp=cnf->root,i=1; Cp; Cp=Cp->next,i++) 
	{
        printf("��%d���Ӿ�: ",i);
        for(Lp=Cp->head,flag=0; Lp; Lp=Lp->next)
		{
            b=cnf->boolarray[(ABS(Lp->literal))];
            if(ASSIGN(Lp->literal)==b) 
			{
                printf("��");
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
    printf("��ӡջ\n");
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
//DPLL�ݹ���⺯��

boolean DPLL_Rec_1(CNF_Solver *cnf, const int f)
{
    int l;
    Clause *Cp;
    Stack st;
    st.next=NULL;
    if(f==0) {
        startTime=clock();
    }
    while((Cp=Find_Unit_Clause(cnf))!=NULL)  //���Ӿ䴫��
	{
        Boolarray_Assign(cnf,Cp->head->literal,f);  //��¼������
        Simplify_UnitClause_1(cnf, Cp->head->literal, &st);  //�����Ӿ�
        if(Have_Empty_Clause(cnf)==TRUE)  //����п��Ӿ�
		{
            Recover_Change(cnf,&st,-1,NULL);
            Backtrack_Assign(cnf,f);
            if(clock()-startTime>=TIMEOUT) 
			{
                printf("��ʱ >%dms\n",TIMEOUT);
                return NOTSURE;
            }
            return FALSE;
        } else if(Have_Clause(cnf)==FALSE)  //���û���Ӿ�
		{
            Recover_Change(cnf,&st,-1,NULL);
            return TRUE;
        }
    }//while
    l=Combine_Strategy(cnf,mom);  //�ۺϲ���ѡ����Ԫ
    Boolarray_Assign(cnf,l,f);  //��¼������
    Add_Clause(cnf,1,&l);  //��ӵ��Ӿ伯��
    Save_Change(&st,SPLIT,cnf->root,NULL);
    int s=DPLL_Rec_1(cnf, f+1);  //�ݹ�
    if(s==TRUE)  //�������
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
    l=-l;  //ȡ��ֵ
    Boolarray_Assign(cnf,l,f);  //��¼������
    Add_Clause(cnf,1,&l);  //��ӵ��Ӿ伯��
    Save_Change(&st,SPLIT,cnf->root,NULL);
    s=DPLL_Rec_1(cnf, f+1);  //�ݹ�
    Recover_Change(cnf,&st,-1,NULL);
    if(s==FALSE) Backtrack_Assign(cnf,f);
    return s;
}

boolean DPLL_Rec_2(CNF_Solver *cnf,const int f)
{
    int s,l;
    int count=0;
    int ret=f;  //���ݲ���
    Clause *Cp;
    Stack st;
    st.floor=f;
    st.next=NULL;
    //VSIDS��ÿ����������һ��������
	//��������ĳ���Ӿ��г��֣�����ԭʼ�����е��Ӿ��ѧϰ�Ӿ䣬�����ֶ�Ӧ�ļ������ͼ�1
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
            //��������������԰���Щû���Ƴ����ֵġ��ܳ���ѧϰ�Ӿ�ɾ��
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
        while((Cp=Find_Unit_Clause(cnf))!=NULL)  //���Ӿ䴫��
		{
            Boolarray_Assign(cnf,Cp->head->literal,f);  //��¼������
            Simplify_UnitClause_2(cnf, Cp, &st);  //�����Ӿ䲢�����޸�
            if(Have_Empty_Clause(cnf)==TRUE)  //����п��Ӿ�
			{
                if(clock()-startTime>=TIMEOUT) 
				{
                    printf("��ʱ >%dms\n",TIMEOUT);
                    return -1-f;
                }
                int *a=(int*)malloc(sizeof(int)*(cnf->literal_num+3));
                Recover_Change(cnf, &st,-1, a);  //��ԭ����ʼ��
                //���ȡֵ������������Ҫ���ѧϰ�Ӿ䲢��ʱ�����
                ret=a[0];
                free(a);
                Backtrack_LearnClause(cnf,f);
                Backtrack_Assign(cnf,f);  //��ֵ����
                if(count>MAXC) 
				{
                    MAXC++;
                    //printf("������...\n");
                    restart=TRUE;
                    //ѧϰ�Ӿ�Ļ���
                    return 0-f;
                }
                return ret-f;
            } 
			else if(Have_Clause(cnf)==FALSE)  //���û���Ӿ�
			{
                Recover_Change(cnf, &st,-1, NULL);//��ԭ����ʼ��
                //ѧϰ�Ӿ�Ļ���
                Backtrack_LearnClause(cnf, f);
                return TRUE;
            }
        }//while
        
        l=Combine_Strategy(cnf,mom);  //�ۺϲ���ѡ����Ԫ

        Add_Clause(cnf,1,&l);
        Save_Change(&st,SPLIT,cnf->root,NULL);  //����������

        //printf("�ݹ�:%d, ѡ%d\n",f,l);

        s=DPLL_Rec_2(cnf,f+1);
        Recover_Change(cnf,&st,1,NULL);
    } while(s==FALSE);  //do-while
    if(s==TRUE) 
	{
        Recover_Change(cnf,&st,-1,NULL);
        Backtrack_LearnClause(cnf,f);  //ѧϰ�Ӿ�Ļ���
        return TRUE;
    }
    else  //������ֵ������
	{
        Recover_Change(cnf,&st,-1,NULL);
        Backtrack_LearnClause(cnf,f);  //ѧϰ�Ӿ�Ļ���
        Backtrack_Assign(cnf,f);  //��ֵ����
        return s+1;
    }
}
