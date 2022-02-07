
#include "ast.h"

LF_PTR mk_lf(char c, AST_PTR exp)
{
	LF_PTR lf = safe_allocate(sizeof(LF));
	lf->exp = exp;
	lf->symbol = c;
	lf->next = NULL;
	return lf;
}

LF_PTR lf_clone(LF_PTR source)
{
	LF_PTR target, tmp;
	if (source == NULL) return NULL;
	tmp = target = mk_lf(source->symbol, source->exp);
	while (source->next != NULL) {
		source = source->next;
		tmp->next = mk_lf(source->symbol, source->exp);
		tmp = tmp->next;
	}
	return  target;
}

/*NFA连接*/
LF_PTR lf_union(LF_PTR lf1, LF_PTR lf2)
{
	LF_PTR tmp;
	tmp = lf1 = lf_clone(lf1);
	lf2 = lf_clone(lf2);

	if (lf1 == NULL) return lf2;
	while (tmp != NULL) {
		if (tmp->next == NULL) {
			tmp->next = lf2;
			break;
		}
		tmp = tmp->next;
	}
	return lf1;
}

/*DFA连接含提取重组*/
LF_PTR lf_union_plus(LF_PTR lf1, LF_PTR lf2)
{
	LF_PTR head, tmp, new, lf;
	head = lf = lf_clone(lf1);

	if (lf1 == NULL) return lf2;

	while (lf2 != NULL) {
		tmp = lf;
		while (tmp != NULL) {
			if (tmp->symbol == lf2->symbol) {
				tmp->exp = arrangeOpNode(Or, tmp->exp, lf2->exp);
				goto NEXT;
			}
			tmp = tmp->next;
		}
		new = mk_lf(lf2->symbol, lf2->exp);
		new->next = head;
		head = new;
	NEXT:
		lf2 = lf2->next;
	}
	return head;
}

void lf_alphabet(AST_PTR exp)
{
	exp->lf = mk_lf(exp->exp_string[0], EPSILON);
}

/*NFA合并*/
LF_PTR lf_concate(LF_PTR lf, AST_PTR exp)
{
	LF_PTR tmp;
	tmp = lf = lf_clone(lf);
	if (tmp == NULL) return NULL;
	while (tmp != NULL) {
		tmp->exp = mkOpNode(Seq, tmp->exp, exp);
		tmp = tmp->next;
	}
	return lf;
}

/*DFA含已提取的合并*/
LF_PTR lf_concate_plus(LF_PTR lf, AST_PTR exp)
{
	LF_PTR tmp;
	tmp = lf = lf_clone(lf);
	if (tmp == NULL) return NULL;
	while (tmp != NULL) {
		tmp->exp = arrangeSeqNode(tmp->exp, exp);
		tmp = tmp->next;
	}
	return lf;
}

int lf_length(LF_PTR lf)
{
	int len = 0;
	while (lf != NULL) {
		len++;
		lf = lf->next;
	}
	return len;
}

int is_minus(AST_PTR exp)
{
	if (exp == NULL) return 0;
	switch (exp->op) {
	case Diff: return 1;
	case Empty: case Epsilon:
	case Alpha:
		return 0;
	case Star:   return is_minus(exp->lchild);
	default: return is_minus(exp->lchild) || is_minus(exp->rchild);
	}
}

#define TABLEMAX 256
AST_PTR* StateTable = NULL; /* store all state */
int next_state = 0;


void addstate(AST_PTR exp)
{
	printf("%s,%d\n", exp->exp_string, exp->state);
	if (exp->state >= 0) return;
	if (next_state % TABLEMAX == 0)
		StateTable = safe_reallocate(StateTable,
			sizeof(exp) * (next_state + TABLEMAX));
	exp->state = next_state;
	printf("\t%s state %d, next_state %d, lf %d", exp->exp_string, exp->state, next_state, &*exp->lf);
	StateTable[next_state++] = exp;
	printf(" **** next_state %d\n", next_state);
	return;
}



AST_PTR EMPTY;
AST_PTR EPSILON;

static LF_PTR(*union_method)() = lf_union;
static LF_PTR(*seq_method)() = lf_concate;

/*递归求导*/
LF_PTR derivative(AST_PTR exp/*, int is_dfa*/) {
	LF_PTR lf;
	if (exp->op == Epsilon) {
		return mk_lf(' ', mkEmpty());
	}
	if (exp->op == Empty) {
		return mk_lf(' ', mkEmpty());
	}
	if (exp->op == Alpha) {
		return mk_lf(*exp->exp_string, mkEpsilon());
	}
	if (exp->op == Or) {
		return union_method(derivative(exp->lchild), derivative(exp->rchild));
	}
	if (exp->op == Seq) {
		if (exp->lchild->nullable == 0) {
			return seq_method(derivative(exp->lchild), exp->rchild);
		}
		else {
			return union_method(seq_method(derivative(exp->lchild), exp->rchild), derivative(exp->rchild));
		}
	}
	if (exp->op == Star) {
		return seq_method(derivative(exp->lchild), exp);
	}
}

void linear_form(AST_PTR exp, int stated)
{
	/* TODO */
	addstate(exp);
	LF_PTR lf = derivative(exp);
	exp->lf = lf;
	//LF_PTR head = lf;
	//while (head != NULL) {
	//	printf("%s\n", head->exp->exp_string);
	//	head = head->next;
	//}//展示本次求导结果
	while (lf != NULL) {
		if (lf->exp->state < 0) {
			linear_form(lf->exp, stated);
		}
		lf = lf->next;
	}
}

/* destroy all lf and state flag */
void reset_hash()
{
	int i;
	for (i = 0; i < HASHSIZE; i++) {
		EXPTAB current = exptab[i];
		while (current != NULL) {
			current->exp->state = -1;
			current->exp->lf = NULL;
			current = current->next;
		}
	}
}


void reg2fa(AST_PTR exp, int is_dfa)
{
	char Q = is_dfa ? 'D' : 'Q';
	char* gv_file_name = is_dfa ? "dfa.gv" : "nfa.gv";

	if (is_dfa) {
		union_method = lf_union_plus;
		if (!is_minus(exp)) seq_method = lf_concate_plus;
		reset_hash();
		free(StateTable);
		next_state = 0;
		StateTable = NULL;
	}
	addstate(EMPTY);
	addstate(exp);
	linear_form(exp, 1);
	printnfa(Q);
	graphviz_nfa(gv_file_name, Q);
}

void reg2nfa(AST_PTR exp)
{
	EMPTY = mkEmpty();
	EPSILON = mkEpsilon();
	if (!is_minus(exp)) reg2fa(exp, 0);
	reg2fa(exp, 1);
	nmdfa();
	printnfa('M');
	graphviz_nfa("mdfa.gv", 'M');
}


static struct Equiv { AST_PTR left, right; } *EquivTable;

int equiv_next;
int EQUIVTABLEMAX = 0;

static void enqueue_equiv(AST_PTR L, AST_PTR R) {
	AST_PTR Q;
	int i;
	if (L == R) return;
	if (L->state > R->state) {
		Q = L, L = R, R = Q;
		/* the first entry of pair is always has small state number */
	}
	for (i = 0; i < equiv_next; i++)
		if (L == EquivTable[i].left && R == EquivTable[i].right) return;
	if (equiv_next == EQUIVTABLEMAX) {
		EQUIVTABLEMAX += TABLEMAX;
		EquivTable = safe_reallocate(EquivTable,
			sizeof(*EquivTable) * EQUIVTABLEMAX);
	}
	EquivTable[equiv_next].left = L;
	EquivTable[equiv_next++].right = R;
	return;
}

/* minimization DFA */
void nmdfa(void) {
	int renumber, i, j, current_queue;
	AST_PTR Qi, Qj, L, R;
	LF_PTR Li;

	for (i = 1; i < next_state; i++) {
		Qi = StateTable[i];
		if (Qi->state < i) continue;
		for (j = i + 1; j < next_state; j++) {

			Qj = StateTable[j];
			if (Qj->state < j) continue;
			equiv_next = 0;
			enqueue_equiv(Qi, Qj);
			for (current_queue = 0; current_queue < equiv_next; current_queue++) {
				L = EquivTable[current_queue].left;
				R = EquivTable[current_queue].right;
				if (L->nullable != R->nullable) goto DISTINGUISHED;
				if (L->lf == NULL && R->lf != NULL) goto DISTINGUISHED;
				if (lf_length(L->lf) != lf_length(R->lf)) goto DISTINGUISHED;
				for (Li = L->lf; Li != NULL; Li = Li->next) {
					LF_PTR Lj;
					for (Lj = R->lf; Lj != NULL; Lj = Lj->next) {
						if (Li->symbol == Lj->symbol) {
							enqueue_equiv(Li->exp, Lj->exp);
							break;
						}
					}
					if (Lj == NULL)
						goto DISTINGUISHED; /* there is no entry j in i */
				}
			}
			for (current_queue = 0; current_queue < equiv_next; current_queue++) {
				printf("FOUND EQUIV D%d = D%d\n",
					EquivTable[current_queue].left->state,
					EquivTable[current_queue].right->state);
				if (EquivTable[current_queue].right->state > EquivTable[current_queue].left->state)
					EquivTable[current_queue].right->state =
					EquivTable[current_queue].left->state;
				else
					EquivTable[current_queue].left->state =
					EquivTable[current_queue].right->state;

			}
		DISTINGUISHED: continue;
		}
	}
	free(EquivTable);
	renumber = 0;
	for (i = 0; i < next_state; i++) {
		Qi = StateTable[i];
		Qi->state = (Qi->state < i) ? StateTable[Qi->state]->state : renumber++;
	}
}


void printnfa(char Q_or_D)
{
	int renumber, i, q;
	AST_PTR Q;
	LF_PTR lf_tmp;
	renumber = 1;
	for (i = 1; i < next_state; i++) {
		Q = StateTable[i];
		if (Q->state < renumber) continue;
		renumber++;

		printf("%c%d =", Q_or_D, Q->state);
		if (Q->nullable) printf(" 1 |");
		else printf(" 0 |");
		lf_tmp = Q->lf;
		while (lf_tmp != NULL) {
			q = lf_tmp->exp->state;
			if (q) printf("| %c %c%d ", lf_tmp->symbol, Q_or_D, q);
			lf_tmp = lf_tmp->next;
		}
		printf("| %c%d = %s", Q_or_D, Q->state, Q->exp_string);
		printf("\n");
	}
}

/* generate graphviz dot format of DFA */
void graphviz_nfa(char* filename, char Q_or_D)
{
	FILE* gv_file;
	int renumber, i, q;
	AST_PTR Q;
	LF_PTR lf;

	if (next_state == 1) {
		printf("it's an empty fa!\n");
		return;
	}

	if ((gv_file = fopen(filename, "w")) == NULL) {
		printf("coudn't create output file %s!\n", filename);
		exit(1);
	}
	fprintf(gv_file, "digraph\n  G { label=\"%s of %s\";\n \nrankdir=LR\n\
start[shape =none]\nstart ->node_1\n",
Q_or_D == 'Q' ? "NFA" : (Q_or_D == 'D' ? "DFA" : "MDFA"),
StateTable[1]->exp_string);

	renumber = 1;
	for (i = 1; i < next_state; i++) {
		Q = StateTable[i];
		if (Q->state < renumber) continue;
		renumber++;

		q = Q->state;

		fprintf(gv_file, "node_%d [label=\"%c%d\"", q, Q_or_D, q);

		if (Q->nullable)
			fprintf(gv_file, ", shape = doublecircle]\n");
		else
			fprintf(gv_file, ", shape = circle]\n");
		lf = Q->lf;
		while (lf != NULL) {
			if (lf->exp != EMPTY)
				fprintf(gv_file, "node_%d -> node_%d [label =\"%c\"]\n",
					q, lf->exp->state, lf->symbol);
			lf = lf->next;
		}
	}

	fprintf(gv_file, "}\n");
	fclose(gv_file);
}
