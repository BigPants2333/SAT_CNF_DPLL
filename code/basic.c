#include "basic.h"

//===========================================================================================
//文字相关函数

status Add_Literal(Clause *Cp, int literal)
{
    Literal *p = (Literal *)malloc(sizeof(Literal));
    if (!p) return ERROR;
    p->literal = literal;
    p->next = Cp->head;
    Cp->head = p;
    Cp->length++;
    return OK;
}

Literal *Remove_Literal(Clause *Cp, int literal)
{
    Literal *p, *q = NULL;
    for (p = Cp->head; p; p = p->next) 
	{
        if (p->literal == literal) 
		{
            Cp->length--;
            if (!q) Cp->head = p->next;
            else q->next = p->next;
            p->next = Cp->rmv;
            Cp->rmv = p;
            return p;
        }
        q = p;
    }
    return NULL;
}

status Recover_Literal(Clause *Cp, Literal *Lp)
{
    Literal *Lq = NULL;
    if (Cp->rmv == Lp) Cp->rmv = Lp->next;
    else 
	{
        for (Lq = Cp->rmv; Lq && Lq->next != Lp; Lq = Lq->next) ;
        if (Lq == NULL) return ERROR;
        Lq->next = Lp->next;
    }
    Lp->next = Cp->head;
    Cp->head = Lp;
    Cp->length++;
    return OK;
}

status Delete_Literal(Clause *Cp, int literal)
{
    Literal *p, *q = NULL;
    for (p = Cp->head; p; p = p->next)
	{
        if (p->literal == literal) 
		{
            Cp->length--;
            if (!q) Cp->head = p->next;
            else q->next = p->next;
            free(p);
            return OK;
        }
        q = p;
    }
    return ERROR;
}

status Boolarray_Assign(CNF_Solver *cnf, int literal, int f)
{
    cnf->boolarray[ABS(literal)] = ASSIGN(literal);
    cnf->floorarray[ABS(literal)] = f;
    return OK;
}

//===========================================================================================
//文字索引相关函数

status Create_Literal_Index(CNF_Solver *cnf)
{
    if (cnf->clause_num == 0 || cnf->literal_num == 0) return FALSE;
    if (cnf->lindex != NULL) Delete_All_Literal_Index(cnf);
    else cnf->lindex = (LiteralIndex *)malloc((cnf->literal_num + 1) * sizeof(LiteralIndex));
    int i;
    Clause *Cp;
    for (i = 1; i <= cnf->literal_num; i++) cnf->lindex[i].next = NULL;
    for (Cp = cnf->root; Cp; Cp = Cp->next) Add_Literal_Index(cnf, Cp);
    return OK;
}

status Add_Literal_Index(CNF_Solver *cnf, Clause *Cp)
{
    if (cnf->lindex == NULL) return FALSE;
    Literal *Lp;
    LiteralIndex *li;
    for (Lp = Cp->head; Lp; Lp = Lp->next) 
	{
        li = (LiteralIndex *)malloc(sizeof(LiteralIndex));
        li->Lp = Lp;
        li->Cp = Cp;
        li->next = cnf->lindex[ABS(Lp->literal)].next;
        cnf->lindex[ABS(Lp->literal)].next = li;
    }
    return OK;
}

status Delete_Clause_Literal_Index(CNF_Solver *cnf, Clause *Cp)
{
    Literal *Lp;
    LiteralIndex *li;
    LiteralIndex *tmp;
    for (Lp = Cp->head; Lp; Lp = Lp->next) 
	{
        for (li = &(cnf->lindex[ABS(Lp->literal)]); li->next->Cp != Cp; li = li->next);
        tmp = li->next;
        li->next = tmp->next;
        free(tmp);
    }
    for (Lp = Cp->rmv; Lp; Lp = Lp->next) 
	{
        for (li = &(cnf->lindex[ABS(Lp->literal)]); li->next->Cp != Cp; li = li->next);
        tmp = li->next;
        li->next = tmp->next;
        free(tmp);
    }
    return OK;
}

status Delete_All_Literal_Index(CNF_Solver *cnf)
{
    int i;
    if (cnf->lindex == NULL) return OK;
    for (i = 1; i <= cnf->literal_num; i++) 
	{
        while (cnf->lindex[i].next) 
		{
            LiteralIndex *li = cnf->lindex[i].next;
            cnf->lindex[i].next = li->next;
            free(li);
        }
    }
    return OK;
}

status Print_Literal_Index(CNF_Solver *cnf)
{
    int i;
    LiteralIndex *li;
    printf("索引：\n");
    for (i = 1; i <= cnf->literal_num; i++) 
	{
        printf("%d: ", i);
        for (li = cnf->lindex[i].next; li; li = li->next) printf("%p ", li->Cp);
        printf("\n");
    }
    return OK;
}

//===========================================================================================
//子句相关函数

status Add_Clause(CNF_Solver *cnf, int n, int *clause)
{
    int i;
    Clause *newClause = (Clause *)malloc(sizeof(Clause));
    if (!newClause || n <= 0) return ERROR;
    newClause->head = NULL;
    newClause->rmv = NULL;
    newClause->length = 0;
    newClause->weight = 1;
    newClause->isremoved = FALSE;
    for (i = 0; i < n; i++) 
	{
        if (!Add_Literal(newClause, *(clause+i))) return ERROR;
    }
    if (cnf->root) cnf->root->prev = newClause;
    newClause->next = cnf->root;
    newClause->prev = NULL;

    cnf->root = newClause;
    cnf->clause_num++;

    return OK;
}

status Insert_Clause_Top(CNF_Solver *cnf, Clause *Cp)
{
    if (cnf->root) cnf->root->prev = Cp;
    Cp->isremoved = FALSE;
    Cp->next = cnf->root;
    Cp->prev = NULL;
    cnf->root = Cp;
    cnf->clause_num++;
    return OK;
}

Clause *Remove_Clause(CNF_Solver *cnf, Clause *Cp)
{
    Clause *p = Cp->prev;
    if (Cp->next) Cp->next->prev = p;
    if (p) p->next = Cp->next;
    else cnf->root = Cp->next;
    Cp->isremoved = TRUE;
    cnf->clause_num--;
    return Cp;
}

status Delete_Clause(CNF_Solver *cnf, Clause *Cp)
{
    Literal *lp;
    Clause *p = Cp->prev;
    if (Cp->next) Cp->next->prev = p;
    if (p) p->next = Cp->next;
    else cnf->root = Cp->next;
    for (lp = Cp->head; lp; lp = Cp->head) 
	{
        Cp->head = lp->next;
        free(lp);
    }
    for (lp = Cp->rmv; lp; lp = Cp->rmv)
	 {
        Cp->rmv = lp->next;
        free(lp);
    }
    free(Cp);
    cnf->clause_num--;
    return OK;
}

status Delete_All_Clause(CNF_Solver *cnf)
{
    Clause *p;
    LearnClause *lcp;
    for (p = cnf->root; p; p = cnf->root) Delete_Clause(cnf, p);
    for (lcp = cnf->learn_root; lcp; lcp = cnf->learn_root) 
	{
        cnf->learn_root = lcp->next;
        free(lcp);
    }
    return OK;
}

status Delete_LearnClause(CNF_Solver *cnf, LearnClause *Lcp)
{
    LearnClause *lcq = cnf->learn_root;
    if (Lcp == cnf->learn_root) cnf->learn_root = Lcp->next;
    else 
	{
        for (lcq = cnf->learn_root; lcq; lcq = lcq->next) 
		{
            if (lcq->next == Lcp)
			{
                lcq->next = Lcp->next;
                break;
            }
        }
    }
    Delete_Clause(cnf, Lcp->clause);
    free(Lcp);
    return OK;
}

status Print_Clause(CNF_Solver *cnf)
{
    printf("-------------------------------------------------------\n");
    printf("literal_num: %d\n", cnf->literal_num);
    printf("clause_num: %d\n", cnf->clause_num);
    printf("boolarray: 0x%p\n", cnf->boolarray);
    int i;
    for (i = 1; i <= cnf->literal_num; i++) 
	{
        printf("%d:%d, ", i, cnf->boolarray[i]);
        if (i % 10 == 0) printf("\n");
    }
    printf("\nfloorarray: 0x%p\n", cnf->floorarray);
    for (i = 1; i <= cnf->literal_num; i++) 
	{
        printf("%d:%d, ", i, cnf->floorarray[i]);
        if (i % 10 == 0) printf("\n");
    }
    Clause *Cp;
    Literal *Lp;
    printf("\n");

    printf("clause:\n");
    for (Cp = cnf->root, i = 1; Cp; Cp = Cp->next, i++) 
	{
        printf("%d:%p length: %d prev:%p next:%p\n", i, Cp, Cp->length, Cp->prev, Cp->next);
        for (Lp = Cp->head; Lp; Lp = Lp->next) printf("%d ", Lp->literal);
        if (Cp->rmv) 
		{
            printf("|| ");
            for (Lp = Cp->rmv; Lp; Lp = Lp->next) printf("%d ", Lp->literal);
        }
        printf("\n");
    }
    return OK;
}

status Print_LearnClause(CNF_Solver *cnf)
{
    printf("-------------------------------------------------------\n");
    printf("learn clause:\n");
    int i;
    LearnClause *lcp;
    Clause *Cp;
    Literal *Lp;
    for (lcp = cnf->learn_root, i = 1; lcp; lcp = lcp->next, i++) 
	{
        Cp = lcp->clause;
        printf("%d:%p length: %d isInStack:%s 决策层:%d\n", i, Cp, 
		        Cp->length, (lcp->isInStack == TRUE) ? "TRUE" : "FALSE", lcp->floor);
        for (Lp = Cp->head; Lp; Lp = Lp->next) printf("%d ", Lp->literal);
        if (Cp->rmv) 
		{
            printf("|| ");
            for (Lp = Cp->rmv; Lp; Lp = Lp->next) printf("%d ", Lp->literal);
        }
        printf("\n");
    }
    return OK;
}

boolean Is_Clause_Empty(CNF_Solver *cnf)
{
    return (cnf->clause_num) ? FALSE : TRUE;
}

boolean Is_Unit_Clause(Clause *Cp)
{
    return (Cp->length == 1) ? TRUE : FALSE;
}

boolean Evaluate_Clause(Clause *Cp, int literal)
{
    int flag = NOTCONTAIN;
    Literal *p;
    for (p = Cp->head; p; p = p->next) 
	{
        if (literal == p->literal) return TRUE;
        else if (-literal == p->literal) flag = NOTSURE;
    }
    return flag;
}

boolean Have_Empty_Clause(CNF_Solver *cnf)
{
    Clause *p;
    for (p = cnf->root; p; p = p->next)
	{
        if (!(p->length)) return TRUE;
    }
    return FALSE;
}

boolean Have_Clause(CNF_Solver *cnf)
{
    return (cnf->clause_num) ? TRUE : FALSE;
}

Clause *Find_Unit_Clause(CNF_Solver *cnf)
{
    Clause *p;
    for (p = cnf->root; p; p = p->next) 
	{
        if (Is_Unit_Clause(p) == TRUE) return p;
    }
    return NULL;
}

//===========================================================================================
//求解器相关函数

status Create_Solver(struct Solver *solver, int literal_num, int clause_num)
{
    solver->literal_num = literal_num;
    solver->clause_num = clause_num;
    solver->root = NULL;
    solver->learn_root = NULL;
    solver->boolarray = (boolean *)malloc((solver->literal_num + 1) * sizeof(boolean));
    solver->floorarray = (int *)malloc((solver->literal_num + 1) * sizeof(int));
    solver->countarray = (int *)malloc((solver->literal_num * 2 + 1) * sizeof(int));
    solver->lindex = NULL;
    if (solver->boolarray && solver->floorarray) 
	{
        Clear_Solver_Answer(solver);
        return OK;
    } 
	else return ERROR;
}

status Destroy_Solver(struct Solver *solver)
{
    if (!solver) return ERROR;
    Delete_All_Clause(solver);
    Delete_All_Literal_Index(solver);
    free(solver->boolarray);
    free(solver->floorarray);
    free(solver->countarray);
    free(solver->lindex);
    free(solver);
    return OK;
}

status Clear_Solver_Answer(struct Solver *solver)
{
    LearnClause *Lcp = solver->learn_root;
    while (Lcp) 
	{
        Delete_Clause_Literal_Index(solver, Lcp->clause);
        Delete_LearnClause(solver, Lcp);
        Lcp = solver->learn_root;
    }
    int i;
    for (i = 0; i <= solver->literal_num; i++) 
	{
        solver->boolarray[i] = NOTSURE;
        solver->floorarray[i] = -1;
        solver->countarray[i] = solver->countarray[i + solver->literal_num] = 0;
    }
    return OK;
}
