2019���������רҵ����ԭ���2�α����ϰ

XML�﷨�����������

1/ XML������Ƕ�׵�Start Tag��End Tag��ɣ����ķ����£�
 
  XML Grammar:
  tag_list -> STAG tag_list ETAG tag_list
           |  TEXT tag_list
           |  Epsilon
  ���У�STAG, ETAG �� TEXT���ս��(�ʷ������г�Ϊ����(token))

2/ �ʷ�����

   a/ STAG: "<"TAG_NAME White_space ATTLIST">" 
	- TAG_NAME��REΪ: [^/!?][^\t >\n\r/]* 
     	- ATTLISTΪ: [^>]*
      STAG����һ���������ʽ����"/>"��β����ΪEmpty
      Element Tag, ��ʡ����End Tag���磺
      <a b="ccc"/>
      ��ȼ��ڣ�
      <a b="ccc"></a>
      Ϊ�����﷨������Empty Element TagҲ����ΪǶ�׶ԣ��ʷ�����
      ��ʶ��Empty Element Tag��Ӧ�ȷ���STAG��Ȼ�󽫴�"</a>"���֣�����
      ���ص�Input Stream(flex��unput(int c))�������������Ĵʷ�
      ����������һ��ETAG, �ɴ��﷨������Զ��������ƽ���STAG��
      ETAG;
      
   b/ ETAG: "</"[^>]*">"

   c/ TEXT: [^< \n\t][^<\n]*
      
      ����Processing instructions��CDATA sections������"<"��ʼ
      ����û��ETAG��֮��ԣ������ǵķ���������XML���﷨�Ƿ���ȷ
      �����жϣ�����ڴʷ�����ʱ��Ҳ������������ʶ��ΪTEXT,
      ���Ӧ��REΪ��
      - Processing instructions:  "<?"[^<?]+"?>"
      - CDATA sections: "<![CDATA["([^\]]+"]"|[^\]])*"]]>"
   
   d/ ע�͵Ĺ���
      ��"<!--"��ʼ����"-->"������XML��ע�ͣ��ڴʷ�����ʱ��
      �ò��ֵ����ݹ��˵�����ϸ��Ƽ���xml.l(�ò����Ѿ����)��

   e/ DTD�Ĵ���
      ������Ƶ��﷨����������XML�﷨�ṹ���з���(Well-Formed 
      XML Documents)���������Ƿ�����DTD����֤(validation),
      ��ˣ��ʷ��������Ѹò������ݲ����κδ�����˵�����ͬ
      ע��һ������ϸ��Ƽ���xml.l(�ò����Ѿ����)��

3/ ����
   ����ϣ��������﷨������ͬʱ��XML�ṹ�����ṹ����ʽ����������
   �Ա��Ժ�Ĵ����磺

   <A>
     <B>
       FOO1
       <C>
	 FOO2
       </C>
       FOO3
     </B>
     <D>
       FOO4
     </D>
     <E>
     </E>
   </A>
    ��XMLTree(���£�
                               A
              ��-----------|---------��
               B               D           E
               |                 |
     ��----��--��----��   TEXT     
    TEXT   C  TEXT  TEXT 
               |
            TEXT

 Ϊ�˶����������ݽṹ��

  struct XML_TREE {
    char * tag_name;  /* ��ʶ�� */
    char * att_list;  /* ����  */
    int   lineno;     /* ��ʶ���ڵ��к�  */
    int  type;        /* ��ʾ����, PCDATA for TEXT;
                          ELEMENT for general TAG;
                          EMPTY for Empty Element Tag */

    struct List {     /* child list */
      struct XML_TREE * node;
      struct List * next;
    } * child_list;
  };

  typedef struct List List;

  typedef struct XML_TREE Xml_tree;

  ��ز������£�
  - Xml_tree * make_node(int type, char * tag,  char * att, int lineno);
     /* ����һ������㣬��child_list = NULL */
  - List  * make_list( Xml_tree * node);
     /* ������һ��Ԫ��(node)������
  - add_child (List * parent, List * child);
     /* �������еĵ�һ��Ԫ�صĵ�child_list��Ϊchild */
  - void add_list ( List * header, List * tail);
     /* ��������List header��tail */
  - void  print_tree(int level, List * header);
     /* ��XML Tree��������ʽ����� xml�ļ� */

  ��ص�ͷ�ļ���ʵ�ּ�xml_tree.h��xml_tree.c(�Ѹ���Դ��)

4/ �������������:
    c:> xmlparse.exe < sample.xml | more (DOS)
    $ xmlparser < sample | more (linux)
5/ �����嵥
    Makefile		Linux Makefile
    tcmake.mak 		Turbo C Makefile
		        ���÷�ʽ��
			1��C:>path=c:\tc;%path
			2��make -ftcmake.mak
    token.h		���ʱ���ĺ궨��
    xml_tree.h		xml tree���ݽṹ�Ķ����
  			��ز�����ͷ�ļ���
    xml_tree.c		xml_tree��ʵ��(���ṩԴ��)
    xml.l		XML�ʷ�����flexԴ����
    parser.c		�ݹ��½�������
			(��Ҫ��ɣ�)
    xmlparser		linux��������ִ���ļ�(64λ)
    xmlparse.exe	DOS��������ִ���ļ�
    sample.xml		�����ļ�
    REC-xml-20040204.xml XML�淶��Ҳ����������
			��������TCʵģʽ���ڴ������
			��DOS������ʵ�����"Out
			of memory�Ĵ���LINUX��û��
			��Ӧ�Ĵ���
    ����չ�������(XML) 1.0.doc
			XML�淶�ĺ����
    readme.txt		���ļ�

    
6/ �������
 
  a/ ��ɵݹ��½���������List *tag_list(), ����ȷ������XML�ļ�֮��
     ������XML Tree Listָ��;

    (��Դ����parser.c�е� /* ����ɣ�*/ ����)
  
  b/ �����ύ�� mailto:hanfei.wang@gmail.com
     subject: ѧ��(2)
     ����: ������parser.c����
     ddl: ��Ⱥ֪ͨ

7/ �ο�Դ�룺 an open source html parser:
   HTML Tidy (https://github.com/htacg/tidy-html5)
   ��start tag��end tag��ƽ��������Ĵ���

hfwang

2021.09.15.
