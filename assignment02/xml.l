%{
/* Copyright hanfei.wang@gmail.com 2013.04.03 */

#include <string.h>
#include "token.h"
#include "xml_tree.h"

extern Xml_tree * yylval;
static char * stag_name ;
/*
 * By W3CExtensible Markup Language (XML) 1.0 (Third Edition)
 * (http://www.w3.org/TR/2004/REC-xml-20040204/)
 * There are three form of TAG:
 * 
 * 1/ Start Tag:   '<' Name (S Attribute)* S? '>'  
 * 2/ End Tag:    '</' Name S? '>'
 * 3/ Empty-Element Tags:   '<' Name (S Attribute)* S? '/>'
 *
 * Because Empty-Element Tags  are not balanced, 
 * we will trait Empty-Element Tags as a pair of
 * STAG & ETAG. ie. if the scanner see <link a="b"/>,
 * it will return an STAG just as <link a="b"> and unput
 * an ETAG </link> back to input bufffer and make EMPTY
 * in the Xml_tree node 
 */    

%}
%option yylineno
%x doctype
%x comment
%x stag
%x inlinedtd
%x script

%%

"<!DOCTYPE"  BEGIN(doctype)    ;/* Skip DTD Section */

<doctype>"[" BEGIN(inlinedtd)  ;
<doctype>">" BEGIN(INITIAL)    ;
<doctype>[^>]                  ;

<inlinedtd>"]" BEGIN(doctype)  ;
<inlinedtd>[^\]]               ;    

"<!--"    BEGIN(comment)         ;
<comment>"-->"    BEGIN(INITIAL) ;
<comment>.|\n ;

"<"[Ss][Cc][Rr][Ii][Pp][Tt]	BEGIN(script);
<script>"</"[Ss][Cc][Rr][Ii][Pp][Tt][ \t\n\r]*">"	BEGIN(INITIAL);
<script>.|\n	;

"<"[^/!?][^\t >\n\r/]* {
  stag_name = (char *) malloc(yyleng);
  if (yytext[2] == '>')
    Error("Empty start tag at line %d!", yylineno);
 
  strcpy(stag_name, yytext + 1);
  BEGIN(stag);
  }

<stag>[^>]*">" { 
  /* for STAG and Empty-Element Tags */
  char * att_list;
  int tag_type = ELEMENT;

  if (yyleng == 1)
    att_list = NULL;
  else if ( yytext[yyleng -2] == '/' ) { /* empty tag */
    int i;
    att_list = (char *) malloc(yyleng);
    strncpy (att_list, yytext, yyleng -2);
    att_list[yyleng -2] = 0;
    unput('>');
    for (i = strlen(stag_name) -1; i >= 0; i--)
      unput(stag_name[i]);
    unput ('/'); unput ('<');
    tag_type = EMPTY;
    } else {
      att_list = (char *) malloc (yyleng);
      strncpy (att_list, yytext, yyleng - 1);
      att_list[yyleng-1]=0;
      }
 
  yylval = make_node (tag_type, stag_name, att_list, yylineno);
  
  /*  printf("%s %s\n", stag_name, att_list); */
  BEGIN(INITIAL);
  return STAG; 
  }

"</"[^>]*">"   {
  /* End Tag */
  char * c_p;
  char * tag_name = (char *) malloc(yyleng - 1);
  if (yyleng == 3)
    Error("Empty end tag at line %d!", yylineno);
  c_p = strchr(yytext, ' ');
  if (c_p == NULL){
    strncpy(tag_name, yytext + 2, yyleng - 3);
    tag_name[ yyleng - 3 ] = 0;
    }
  else{
    strncpy(tag_name, yytext + 2, c_p - yytext - 2);
    tag_name[c_p-yytext -2] = 0;
   }
  yylval = make_node(ELEMENT, tag_name, NULL, yylineno);

  return ETAG; 
}

(" "|\t)+   ;

([^<\n \t][^<\n]*)|("<?"[^<?]+"?>")|("<![CDATA["([^\]]+"]"|[^\]])*"]]>")  {
  char * att = (char *) malloc (yyleng + 1 );
  strcpy(att, yytext);
  yylval = make_node(PCDATA, NULL, att, yylineno);
  
  return TEXT;
  }


\n      /* printf("%s" , yytext)*/;

%%


int yywrap()

{
        return 1;
}


