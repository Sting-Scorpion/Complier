
all: xmlparser

object = lex.yy.o parser.o xml_tree.o

lex.yy.c: xml.l
	flex xml.l
.c.o:
	gcc -c -g3 $<
xmlparser: ${object}
	gcc -o xmlparser ${object}

${object}: xml_tree.h token.h

clean:
	rm -f *.o lex.yy.c
