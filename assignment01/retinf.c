/* retinf.c   	AXL分析器 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lex.h"

char err_id[] = "error";
char* midexp;
extern char* yytext;

struct YYLVAL {
	char* val;  /* 记录表达式中间临时变量 */
	char* expr; /* 记录表达式后缀式 */
	int last_op;  /* last operation of expression
			 for elimination of redundant parentheses */
};

typedef struct YYLVAL Yylval;

Yylval* expression(void);

char* newname(void); /* 在name.c中定义 */

extern void freename(char* name);

void statements(void)
{
	/*  statements -> expression SEMI  |  expression SEMI statements  */
	/*  请完成！！！*/

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
	/*  请完成！！！*/

	Yylval* temp = (Yylval*)malloc(sizeof(Yylval));

	//加减法直接与符号连接
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
	//乘除法注意括号
	else if (match(TIMES) || match(DIVISION)) {
		char op = yytext[0];
		temp->last_op = op;
		advance();
		Yylval* temp1 = expression();
		Yylval* temp2 = expression();
		temp->val = temp1->val;

		printf("    %s %c= %s\n", temp->val, op, temp2->val);
		freename(temp2->val);

		//判断两个表达式是否含加减号，是否需要括号
		int exp1Parenthesis = (temp1->last_op == '+' || temp1->last_op == '-');
		int exp2Parenthesis = (temp2->last_op == '+' || temp2->last_op == '-');
		int changeOperator = (op == '/' && (temp2->last_op == '*' || temp2->last_op == '/'));
		exp2Parenthesis = exp2Parenthesis || changeOperator;
		int ParenthesisLength = (exp1Parenthesis + exp2Parenthesis) * 2;
		temp->expr = (char*)malloc(strlen(temp1->expr) + 1 + strlen(temp2->expr) + ParenthesisLength + 1);

		int i = 0;
		//如果不需要加括号则下标不会前进，后面内容会覆盖已写入的括号
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

