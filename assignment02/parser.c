/* Copyright hanfei.wang@gmail.com 2013.04.03 */

#include "xml_tree.h"
#include "token.h"
#include <string.h>

extern int yylineno;
extern int yylex();

Xml_tree* yylval;

static lookahead = -1;

static char* tag_stack[100];

static int current_stack = -1;

void push_stack(char* tag)
{
	tag_stack[++current_stack] = strdup(tag);
}

void pop_stack()
{
	current_stack--;
}

int search_stack(char* tag)
{
	int i;
	for (i = current_stack; i >= 0; i--)
		if (strcasecmp(tag_stack[i], tag) == 0)
			return 1;
	return 0;
}

void print_stack()
{
	int i;
	for (i = 0; i <= current_stack; i++)
		fprintf(stderr, "%s\n", tag_stack[i]);
}

int match(int token)
{
	if (lookahead == -1)
		lookahead = yylex();
	return token == lookahead;
}

int advance()
{
	lookahead = yylex();
}


/*
 * XML Grammar:
 * tag_list -> STAG tag_list ETAG tag_list
 *          |  TEXT tag_list
 *          |  Epsilon
 */

List* tag_list()
{
	/* ÇëÍê³É */
	List* l = (List*)malloc(sizeof(List));
	l->next = (List*)malloc(sizeof(List));
	if (match(STAG)) {
		l->node = yylval;
		advance();
		l->node->child_list = tag_list();

		if (match(ETAG)) {
			advance();
			l->next->next = tag_list();
		}
	}
	else if (match(TEXT)) {

		l->node = yylval;
		advance();
		l->next = tag_list();
	}
	else {
		l = NULL;
	}
	return l;
}

int main()
{
	List* root;
	root = tag_list();
	print_tree(0, root);
	return 0;
}
