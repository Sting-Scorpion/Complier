/* Copyright hanfei.wang@gmail.com 2013.04.04 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

/*  3 types of XML Tree node  */
#define PCDATA  0  /* for TEXT */
#define ELEMENT 1  /* for STAG ETAG pair */
#define EMPTY 	2  /* for Empty-Element Tags */

struct XML_TREE {
  char * tag_name;
  char * att_list;
  int   lineno;
  int  type;
  struct List {
    struct XML_TREE * node;
    struct List * next;
  } * child_list;
};

typedef struct List List;

typedef struct XML_TREE Xml_tree;

void Error(const char * fmt, ...);

Xml_tree * make_node(int type, char * tag,  char * att, int lineno);

List  * make_list( Xml_tree * node);

void  add_child (List * parent, List * child);

void  add_list(List * header, List * tail);

void  print_tree(int level, List * header);
