#include "cnfparser.h"

CNF_Solver* Load_CNF(char *path)
{
    int i,j;
    char ch;
    int l[LITERALMAX];
    int literal_num,clause_num;
    FILE *fp;
    fp=fopen(path,"r");
    if(!fp) return NULL;
    CNF_Solver *cnf=(CNF_Solver*)malloc(sizeof(CNF_Solver));
    if(!cnf) return NULL;
    while((ch=fgetc(fp))=='c') 
	{
        while((ch=fgetc(fp))!='\n');
    }
    for(i=0; i<5; i++) ch=fgetc(fp);
    fscanf(fp,"%d",&literal_num);
    fscanf(fp,"%d",&clause_num);
    if(!Create_Solver(cnf,literal_num,0)) 
	{
        fclose(fp);
        return ERROR;
    }
    for(i=0; i<clause_num; i++) 
	{
        j=0;
        do 
		{
            fscanf(fp,"%d",&l[j]);
            j++;
        } while(l[j-1]!=0);
        if(!Add_Clause(cnf,j-1,l)) 
		{
            fclose(fp);
            return NULL;
        }
    }
    fclose(fp);
    return cnf;
}
status To_Save_Path(char *respath, char *cnfpath)
 {
    int i=-1;
    do 
	{
        i++;
        respath[i]=cnfpath[i];
    } while(cnfpath[i]!='\0');
    i-=3;
    respath[i++]='r';
    respath[i++]='e';
    respath[i]='s';
    return OK;
}
status Save_Res(char *respath, status DPLL, CNF_Solver *cnf, int time)
{
    int i;
    FILE *fp=fopen(respath,"w");
    if(!fp) return ERROR;
    fprintf(fp,"s %d\nv",DPLL);
    for(i=1; i<=cnf->literal_num; i++) 
        fprintf(fp," %d",cnf->boolarray[i]*i);
    fprintf(fp,"\nt %d\n",time);
    fclose(fp);
    return OK;
}
status Save_CNF(CNF_Solver* cnf, char *path)
{
    FILE *fp=fopen(path,"w");
    Clause *Cp;
    Literal *Lp;
    if(!fp) return ERROR;
    fprintf(fp,"p cnf %d %d\n",cnf->literal_num,cnf->clause_num);
    for(Cp=cnf->root; Cp; Cp=Cp->next) 
	{
        for(Lp=Cp->head; Lp; Lp=Lp->next) 
		{
            fprintf(fp,"%d ",Lp->literal);
        }
        fprintf(fp,"0\n");
    }
    fclose(fp);
    return OK;
}
