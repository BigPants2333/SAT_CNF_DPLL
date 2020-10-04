#include "binarypuzzle.h"

//===========================================================================================
//限制条件子句生成

status BP_Constraint_1(BP_Solver *bp,int n)
{
	int i,j;
	int pos[3],neg[3];
	for(i=1;i<=n;i++)
	{
		for(j=1;j<=n-2;j++)
		{
			pos[0]=(i-1)*n+j;neg[0]=-pos[0];
			pos[1]=(i-1)*n+j+1;neg[1]=-pos[1];
			pos[2]=(i-1)*n+j+2;neg[2]=-pos[2];
			Add_Clause(bp,3,pos);
			Add_Clause(bp,3,neg);
		}
	}
	for(j=1;j<=n;j++)
	{
		for(i=1;i<=n-2;i++)
		{
			pos[0]=(i-1)*n+j;neg[0]=-pos[0];
			pos[1]=(i-1+1)*n+j;neg[1]=-pos[1];
			pos[2]=(i-1+2)*n+j;neg[2]=-pos[2];
			Add_Clause(bp,3,pos);
			Add_Clause(bp,3,neg);
		}
	}
	return OK;
}

status BP_Constraint_2(BP_Solver *bp,int n)
{
	int i,j,k,m,p;  //m=n/2+1,p=C(n,m)
	m=n/2+1,p=Com(n,m);
	int E_Arr[n],C_Arr[p*m],index[m];
	for(i=0;i<n;i++) E_Arr[i]=i+1;
	IterativeCombos(n,m,C_Arr,E_Arr,index);
	
	int _pos[m],_neg[m];
	for(i=1;i<=n;i++)
	{
		for(j=0;j<p;j++)
		{
			for(k=0;k<m;k++)
			{
				_pos[k]=(i-1)*n+C_Arr[m*j+k];
				_neg[k]=-_pos[k];
			}
			Add_Clause(bp,m,_pos);
			Add_Clause(bp,m,_neg);
		}
	}
	for(j=1;j<=n;j++)
	{
		for(i=0;i<p;i++)
		{
			for(k=0;k<m;k++)
			{
				_pos[k]=(C_Arr[m*i+k]-1)*n+j;
				_neg[k]=-_pos[k];
			}
			Add_Clause(bp,m,_pos);
			Add_Clause(bp,m,_neg);
		}
	}
	return OK;
}

status BP_Constraint_3(BP_Solver *bp,int n)
{
	int i,j,k,
	    _i,_j,
	    count=0,
	    p,q;
	p=n*(n-1)/2,q=p*n,k=0;
	int a_i[q],b_i[q],c_i[q],
	    a_j[q],b_j[q],c_j[q],
		d_i[p],d_j[p];
		
	//添加辅助变量以完成Tseytin变换 
	for(i=1;i<=p;i++)
	{
		for(j=1;j<=n;j++)
		{
			a_i[k]=(i-1)*n+j+n*n;
			b_i[k]=(i-1)*n+j+n*n+q;
			c_i[k]=(i-1)*n+j+n*n+q*2;
			a_j[k]=(i-1)*n+j+n*n+q*3;
			b_j[k]=(i-1)*n+j+n*n+q*4;
			c_j[k]=(i-1)*n+j+n*n+q*5;
			k++;
		}
	}
	for(i=1;i<=p;i++) 
	{
		d_i[i-1]=c_j[q-1]+i;
		d_j[i-1]=d_i[i-1]+p;
	}
	
	int Tseytin_1[3],Tseytin_2[2],
	    Tseytin_3[2],Tseytin_4[2];
	int c_d[n+1];	
	
	//无重复行
	while(count<p)
	{
		for(i=1;i<=n;i++)
		{
			for(_i=i+1;_i<=n;_i++)
			{
				for(j=1;j<=n;j++)
				{
					//添加a_i语句 
					Tseytin_1[0]=a_i[count*n+j-1];
					Tseytin_2[0]=Tseytin_3[0]=-Tseytin_1[0];
					Tseytin_1[1]=-((i-1)*n+j),Tseytin_2[1]=-Tseytin_1[1];
					Tseytin_1[2]=-((_i-1)*n+j),Tseytin_3[1]=-Tseytin_1[2];
					Add_Clause(bp,3,Tseytin_1);
					Add_Clause(bp,2,Tseytin_2);
					Add_Clause(bp,2,Tseytin_3);
					
					//添加b_i语句
					Tseytin_1[0]=b_i[count*n+j-1];
					Tseytin_2[0]=Tseytin_3[0]=-Tseytin_1[0];
					Tseytin_1[1]=(i-1)*n+j,Tseytin_2[1]=-Tseytin_1[1];
					Tseytin_1[2]=(_i-1)*n+j,Tseytin_3[1]=-Tseytin_1[2];
					Add_Clause(bp,3,Tseytin_1);
					Add_Clause(bp,2,Tseytin_2);
					Add_Clause(bp,2,Tseytin_3);
					
					//添加c_i语句
					Tseytin_1[0]=-c_i[count*n+j-1],
					Tseytin_1[1]=a_i[count*n+j-1],
					Tseytin_1[2]=b_i[count*n+j-1];
					Tseytin_2[0]=Tseytin_3[0]=-Tseytin_1[0];
					Tseytin_2[1]=-Tseytin_1[1];
					Tseytin_3[1]=-Tseytin_1[2];
					Add_Clause(bp,3,Tseytin_1);
					Add_Clause(bp,2,Tseytin_2);
					Add_Clause(bp,2,Tseytin_3);
					
					//添加d_i语句，部分一
					Tseytin_4[0]=c_i[count*n+j-1];
					Tseytin_4[1]=d_i[count];
					Add_Clause(bp,2,Tseytin_4);
					
					c_d[j-1]=-c_i[count*n+j-1];
				}
				//添加d_i语句，部分二 
				c_d[j-1]=-d_i[count];
				Add_Clause(bp,n+1,c_d);
				count++;
			}
		}
	}
	
	count=0; 
	//无重复列 
	while(count<p)
	{
		for(j=1;j<=n;j++)
		{
			for(_j=j+1;_j<=n;_j++)
			{
				for(i=1;i<=n;i++)
				{
					//添加a_j语句 
					Tseytin_1[0]=a_j[count*n+i-1];
					Tseytin_2[0]=Tseytin_3[0]=-Tseytin_1[0];
					Tseytin_1[1]=-((i-1)*n+j),Tseytin_2[1]=-Tseytin_1[1];
					Tseytin_1[2]=-((i-1)*n+_j),Tseytin_3[1]=-Tseytin_1[2];
					Add_Clause(bp,3,Tseytin_1);
					Add_Clause(bp,2,Tseytin_2);
					Add_Clause(bp,2,Tseytin_3);
					
					//添加b_j语句
					Tseytin_1[0]=b_j[count*n+i-1];
					Tseytin_2[0]=Tseytin_3[0]=-Tseytin_1[0];
					Tseytin_1[1]=(i-1)*n+j,Tseytin_2[1]=-Tseytin_1[1];
					Tseytin_1[2]=(i-1)*n+_j,Tseytin_3[1]=-Tseytin_1[2];
					Add_Clause(bp,3,Tseytin_1);
					Add_Clause(bp,2,Tseytin_2);
					Add_Clause(bp,2,Tseytin_3);
					
					//添加c_j语句
					Tseytin_1[0]=-c_j[count*n+i-1],
					Tseytin_1[1]=a_j[count*n+i-1],
					Tseytin_1[2]=b_j[count*n+i-1];
					Tseytin_2[0]=Tseytin_3[0]=-Tseytin_1[0];
					Tseytin_2[1]=-Tseytin_1[1];
					Tseytin_3[1]=-Tseytin_1[2];
					Add_Clause(bp,3,Tseytin_1);
					Add_Clause(bp,2,Tseytin_2);
					Add_Clause(bp,2,Tseytin_3);
					
					//添加d_j语句，部分一
					Tseytin_4[0]=c_j[count*n+i-1];
					Tseytin_4[1]=d_j[count];
					Add_Clause(bp,2,Tseytin_4);
					
					c_d[i-1]=-c_j[count*n+i-1];
				}
				//添加d_j语句，部分二 
				c_d[i-1]=-d_j[count];
				Add_Clause(bp,n+1,c_d);
				count++;
			}
		}
	}
	
	for(i=0;i<p;i++)
	{
		Add_Clause(bp,1,&d_i[i]);
		Add_Clause(bp,1,&d_j[i]);
	}

	return OK;
}

BP_Solver *Create_BP_CNF(int n)
{
	BP_Solver *bp=(BP_Solver *)malloc(sizeof(BP_Solver));
	if(!bp) return NULL;
	int i,size=n*n+n*(n-1)*(3*n+1);
	Create_Solver(bp,size,0);
	
	BP_Constraint_1(bp,n);
	BP_Constraint_2(bp,n);
	BP_Constraint_3(bp,n);
	
	return bp;
}

//===========================================================================================
//处理文件中BP字符串函数

BP_file* Read_BP_File(char *path)
{
	FILE *fp=fopen(path,"r");
	if(!fp) return NULL;
	BP_file *bp_file=(BP_file *)malloc(sizeof(BP_file)); 
	if(!bp_file) return NULL;
	bp_file->head=NULL;
	fscanf(fp,"%d %d",&bp_file->n,&bp_file->num);
	
	int m,i,count;
	char ch;
	m=bp_file->n*bp_file->n;
	for(i=0;i<bp_file->num;i++)
	{
		BP_node *bp_node=(BP_node *)malloc(sizeof(BP_node));
		bp_node->s=(char *)malloc(sizeof(char)*(m+1));
		count=0;
		while(1)
		{
			fscanf(fp,"%c",&ch);
			if(ch=='\n') continue;
			else
			{
				*(bp_node->s+count)=ch;
				count++;
				if(count==m) break;
			} 
		}
		*(bp_node->s+count)='\0';
		bp_node->count=bp_file->num-i;
		bp_node->next=bp_file->head;
		bp_file->head=bp_node;
	}
	fclose(fp);
	return bp_file;
}

status Read_BP_Str(BP_Solver *bp,int n,char *s)
{
	int m=n*n;
	int i;
	int tmp;
	for(i=0;i<m;i++)
	{
		switch(s[i])
		{
			case '0':
				tmp=-(i+1);
				Add_Clause(bp,1,&tmp);
				break;
			case '1':
				tmp=i+1;
				Add_Clause(bp,1,&tmp);
				break;
			default:
				break;
		}
	}
	return OK;
}

char* Solved_BP_Str(BP_Solver *bp,int n)
{
	char *s=(char *)malloc((n*n+1)*sizeof(char));
	int i;
	for(i=1;i<=n*n;i++)
	{
		if(bp->boolarray[i]==1) s[i-1]='1';
		else if(bp->boolarray[i]==-1) s[i-1]='0';
		else if(bp->boolarray[i]==0) s[i-1]='#';
	}
	s[n*n]='\0';
	return s;
}

//===========================================================================================
//生成随机初始棋盘函数 

status Two_Random_Cells(BP_Solver *bp,int n)
{
	int cell_1,cell_2,val_1,val_2;
	cell_1=rand()%(n*n)+1;
	cell_2=rand()%(n*n)+1;
	if(cell_1==cell_2) cell_2=(cell_2+Com(n,((n/2)+1)))%(n*n)+1;
	val_1=rand()%2;
	val_2=rand()%2;
	if(!val_1) cell_1=0-cell_1;
	if(!val_2) cell_2=0-cell_2;
	Add_Clause(bp,1,&cell_1);
	Add_Clause(bp,1,&cell_2);
	return OK; 
}

char* Initialize_BP(BP_Solver *bp,int n)
{
	int res;
	char *s;
	Two_Random_Cells(bp,n);
	Create_Literal_Index(bp);
	res=DPLL_Rec_2(bp,0);
	if(res==TRUE) s=Solved_BP_Str(bp,n);
	return s;
} 

char* Dig_Holes(char *initial,int n)
{
	int lim;
	int i,count,num;
	char *s=(char *)malloc(sizeof(char)*(n*n+1));
	Hole holes[n*n];
	lim=n+Com(n,((n/2)+1));
	
	for(i=0;i<n*n;i++)
	{
		holes[i].mark=0;
		if(initial[i]=='1') holes[i].val=1;
		else holes[i].val=0;
	}
	
	count=0;
	while(1)
	{
		num=rand()%(n*n);
		if(holes[num].mark==0)
		{
			holes[num].mark=1;
			count++;
		}
		else
		{
			holes[num].mark=0;
			count--;
		}
		if(count==lim) break;
	}
	for(i=0;i<n*n;i++)
	{
		if(holes[i].mark==1) s[i]='#';
		else
		{
			if(holes[i].val==0) s[i]='0';
			else s[i]='1';
		}
	}
	s[i]='\0';
	return s;
}

BP_Solver* Create_Random_BP(BP_Solver *bp,int n,char *s)
{
	if(bp)
	{
		Destroy_Solver(bp);
        bp=NULL;
	}
	bp=Create_BP_CNF(n);
	Read_BP_Str(bp,n,s);
	return bp;
}

//===========================================================================================
//辅助函数

status Print_BP_Str(char *bp_str,int n)
{
	int i,m;
	m=n*n;
	for(i=0;i<m;i++)
	{
		printf("%c ",bp_str[i]);
		if((i+1)%n==0) printf("\n");
	}
	return OK;
}

int Fac(int n)
{
	int i=0;
	int sum=1;
	if (n==0)
	{
		return 1;
	}
	for (i=1;i<=n;i++)
	{
		sum*=i;
	}
	return sum;
} 

int Com(int n,int m)
{
	int res;
	res=Fac(n)/(Fac(n-m)*Fac(m));
	return res;
}

void IterativeCombos(int n,int m,int C_Arr[],int E_Arr[],int index[])
{
	int M_FILL=0,M_INC=1;
	int i=0,j=0,k=0;  
	int mode=M_FILL;
	while (i>=0)
	{
		if (mode==M_FILL)	//填充模式
		{
			if (i==0)
				index[0]=0;
			else
				index[i] = index[i-1]+1;
			
			if (i == m-1)	//当前焦点已经达到最大深度
			{
				for(j=0;j<m;j++) //保存至组合数数组中
				{
					C_Arr[k++]=E_Arr[index[j]];
				}
				mode=M_INC;	//切换为增量模式	
			}
			else			//没有达到最大深度
				i++;		//继续填充下级节点
		}
		else				//增量模式
		{
			index[i]++;	//焦点元素递增
			
			if ( index[i] > n-m+i ) //已经超限
				i--;
			else
			{
				if (i==m-1)		//当前焦点已经达到最大深度
					for(j=0;j<m;j++) //保存至组合数数组中 
					{
						C_Arr[k++]=E_Arr[index[j]];
					}
				else
				{
					i++;		 //继续填充下级节点
					mode=M_FILL; //切换到填充模式
				}
			}
		}
	}
}
