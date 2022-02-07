/* Copyright hanfei.wang@gmail.com 2013.04.03 */

/* XML Tree data strutrure */
  
#include "xml_tree.h"
#include <string.h>

void Error(const char * fmt, ...)
{
  va_list argp;
  fprintf(stderr, "Error: ");
  va_start(argp, fmt);
  vfprintf(stderr, fmt, argp);
  va_end(argp);
  fprintf(stderr, "\n");
  /* exit (1); */
}

Xml_tree * make_node(int type,  char * tag,  char * att, int lineno)
{
  Xml_tree * node = (Xml_tree *) malloc(sizeof(Xml_tree));
  if (node == NULL) 
    Error("Out of memory");
  node -> tag_name = tag;
  node -> att_list = att;
  node -> type = type;
  node -> lineno = lineno;
  node -> child_list = NULL;
  return node;
}

List  * make_list( Xml_tree * node)
{
  List * header = (List *) malloc (sizeof(List));
  if (header == NULL)
    Error ( "Out of memory" );
  header -> node = node;
  header -> next = NULL;
  return header;
}

void add_child (List * parent, List * child)
{
  Xml_tree * current_parent = parent -> node;
  current_parent -> child_list = child;
}

void add_list ( List * header, List * tail)
{
  List * cursor = header;
  /*  List * tail_element = tail -> node -> child_list; */
  while( cursor -> next != NULL)
    cursor = cursor -> next;
  cursor -> next = tail;
}

/*
 * following 2 mutually recursive function
 * will travel the generated XML tree and
 * print the tree in an indent format.
 */

void print_tag(int level, Xml_tree * tag)
{
  if (tag == NULL) return;
  if (tag->type == PCDATA) {
    if (tag -> att_list != NULL){
      printf("%*s", level, "");
      printf("%s\n", tag ->att_list);
    }
    return;
  }

  printf ( "%*s", level, "");

  if (tag ->type == EMPTY) {
    if (tag ->att_list == NULL)
      printf ("<%s/>\n", tag->tag_name);
    else
      printf ("<%s %s/>\n", tag->tag_name, tag-> att_list);
    return;
  }

  if (tag ->att_list == NULL)
    printf ("<%s>\n", tag->tag_name);
  else
    printf ("<%s %s>\n", tag->tag_name, tag-> att_list);
  print_tree(level + 2, tag -> child_list);
  printf ( "%*s", level, "");
  printf ("</%s>\n", tag->tag_name);  
}

void print_tree(int level, List * header)
{
  List * cursor = header;
  while (cursor != NULL) {
    print_tag(level, cursor -> node);
    cursor = cursor ->next;
  } 
}
  
  
