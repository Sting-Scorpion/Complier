# You must set TCHOME to your proper TC Home Directory!
TCHOME=E:\TC

CFLAGS	= -v -O 
INCLUDE = -I$(TCHOME)\include;.
LIB = -L$(TCHOME)\lib;
CC	= $(TCHOME)\tcc
MODEL  = -mh

.c.obj:
	 $(CC) $(INCLUDE) -c $(CFLAGS) $(MODEL) $<

OBJ = lexyy.obj xml_tree.obj parser.obj

all: xmlparse.exe

lexyy.obj: lexyy.c

xmlparse.exe: $(OBJ) 
	$(CC) -exmlparse.exe $(LIB) $(MODEL) $(CLIB) $(OBJ)

lexyy.c: xml.l
	flex xml.l



${OBJ} : xml_tree.h token.h

clean:
	del  *.obj 
