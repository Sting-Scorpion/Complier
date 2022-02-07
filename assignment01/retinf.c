/* retinf.c   	AXL������ */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lex.h"

char err_id[] = "error";
char* midexp;
extern char* yytext;

struct YYLVAL {
	char* val;  /* ��¼���ʽ�м���ʱ���� */
	char* expr; /* ��¼���ʽ��׺ʽ */
	int last_op;  /* last operation of expression
			 for elimination of redundant parentheses */
};

typedef struct YYLVAL Yylval;

Yylval* expression(void);

char* newname(void); /* ��name.c�ж��� */

extern void freename(char* name);

void statements(void)
{
	/*  statements -> expression SEMI  |  expression SEMI statements  */
	/*  ����ɣ�����*/

	Yylval* temp;
	printf("Input:");
	advance();
	while (!match(EOI)) {
		temp = expression();
		printf("affix expression: %s\n", temp->expr);
		free(temp->expr);
		freename(temp->val);
		free(temp);
		if (match(SEMI)) {
			printf("next:");
			advance();
		}
		else {
			fprintf(stderr, "%d: Inserting missing semicolon\n", yylineno);
			exit(0);
		}
	}
	printf("Finished.\n");
}

Yylval* expression()
{
	/*
	  expression -> PLUS expression expression
				 |  MINUS expression expression
				 |  TIMES expression expression
				 |  DIVISION expression expression
				 |  NUM_OR_ID
	*/
	/*  ����ɣ�����*/

	Yylval* temp = (Yylval*)malloc(sizeof(Yylval));

	//�Ӽ���ֱ�����������
	if (match(PLUS) || match(MINUS)) {
		char op = yytext[0];
		temp->last_op = op;
		advance();
		Yylval* temp1 = expression();
		Yylval* temp2 = expression();
		temp->val = temp1->val;

		printf("    %s %c= %s\n", temp->val, op, temp2->val);
		freename(temp2->val);

		int changeOperator = (op == '-' && (temp2->last_op == '+' || temp2->last_op == '-'));
		temp->expr = (char*)malloc(strlen(temp1->expr) + 1 + strlen(temp2->expr) + changeOperator + 1);

		int i = 0;
		for (int j = 0; j < strlen(temp1->expr); i++, j++) {
			temp->expr[i] = temp1->expr[j];
		}
		temp->expr[i] = op;
		i++;
		temp->expr[i] = '(';
		i += changeOperator;
		for (int j = 0; j < strlen(temp2->expr); i++, j++) {
			temp->expr[i] = temp2->expr[j];
		}
		temp->expr[i] = ')';
		i += changeOperator;
		temp->expr[i] = '\0';
	}
	//�˳���ע������
	else if (match(TIMES) || match(DIVISION)) {
		char op = yytext[0];
		temp->last_op = op;
		advance();
		Yylval* temp1 = expression();
		Yylval* temp2 = expression();
		temp->val = temp1->val;

		printf("    %s %c= %s\n", temp->val, op, temp2->val);
		freename(temp2->val);

		//�ж��������ʽ�Ƿ񺬼Ӽ��ţ��Ƿ���Ҫ����
		int exp1Parenthesis = (temp1->last_op == '+' || temp1->last_op == '-');
		int exp2Parenthesis = (temp2->last_op == '+' || temp2->last_op == '-');
		int changeOperator = (op == '/' && (temp2->last_op == '*' || temp2->last_op == '/'));
		exp2Parenthesis = exp2Parenthesis || changeOperator;
		int ParenthesisLength = (exp1Parenthesis + exp2Parenthesis) * 2;
		temp->expr = (char*)malloc(strlen(temp1->expr) + 1 + strlen(temp2->expr) + ParenthesisLength + 1);

		int i = 0;
		//�������Ҫ���������±겻��ǰ�����������ݻḲ����д�������
		temp->expr[i] = '(';
		i += exp1Parenthesis;
		for (int j = 0; j < strlen(temp1->expr); i++, j++) {
			temp->expr[i] = temp1->expr[j];
		}
		temp->expr[i] = ')';
		i += exp1Parenthesis;
		temp->expr[i] = op;
		i++;
		temp->expr[i] = '(';
		i += exp2Parenthesis;
		for (int j = 0; j < strlen(temp2->expr); i++, j++) {
			temp->expr[i] = temp2->expr[j];
		}
		temp->expr[i] = ')';
		i += exp2Parenthesis;
		temp->expr[i] = '\0';
	}
	else if (match(NUM_OR_ID)) {
		temp->val = newname();
		temp->expr = (char*)malloc(yyleng + 1);
		temp->expr[yyleng] = '\0';
		for (int i = 0; i < yyleng; i++) {
			temp->expr[i] = yytext[i];
		}

		printf("    %s = %s\n", temp->val, temp->expr);
		advance();
	}
	else {
		fprintf(stderr, "%d Wrong Input. Number or identifier expected\n", yylineno);
	}
	return temp;
}

