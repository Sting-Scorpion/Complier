2019级软件工程专业编译原理第2次编程练习

XML语法分析器的设计

1/ XML语言由嵌套的Start Tag和End Tag组成，其文法如下：
 
  XML Grammar:
  tag_list -> STAG tag_list ETAG tag_list
           |  TEXT tag_list
           |  Epsilon
  其中：STAG, ETAG 和 TEXT是终结符(词法分析中称为单词(token))

2/ 词法规则：

   a/ STAG: "<"TAG_NAME White_space ATTLIST">" 
	- TAG_NAME的RE为: [^/!?][^\t >\n\r/]* 
     	- ATTLIST为: [^>]*
      STAG还有一个特殊的形式：以"/>"结尾，称为Empty
      Element Tag, 它省略了End Tag，如：
      <a b="ccc"/>
      其等价于：
      <a b="ccc"></a>
      为了让语法分析将Empty Element Tag也分析为嵌套对，词法分析
      在识别Empty Element Tag后应先返回STAG，然后将串"</a>"逐字（倒序）
      返回到Input Stream(flex的unput(int c))，这样接下来的词法
      分析将返回一个ETAG, 由此语法分析永远看到的是平衡的STAG和
      ETAG;
      
   b/ ETAG: "</"[^>]*">"

   c/ TEXT: [^< \n\t][^<\n]*
      
      由于Processing instructions和CDATA sections都是以"<"开始
      但是没有ETAG与之配对，而我们的分析器仅就XML的语法是否正确
      进行判断，因此在词法分析时，也将这两个部分识别为TEXT,
      其对应的RE为：
      - Processing instructions:  "<?"[^<?]+"?>"
      - CDATA sections: "<![CDATA["([^\]]+"]"|[^\]])*"]]>"
   
   d/ 注释的过滤
      以"<!--"开始并以"-->"结束是XML的注释，在词法分析时将
      该部分的内容过滤掉，详细设计见：xml.l(该部分已经完成)。

   e/ DTD的处理
      我们设计的语法分析器仅对XML语法结构进行分析(Well-Formed 
      XML Documents)，不进行是否满足DTD的认证(validation),
      因此，词法分析将把该部分内容不做任何处理过滤掉，如同
      注释一样，详细设计见：xml.l(该部分已经完成)。

3/ 语义
   我们希望在完成语法分析的同时将XML结构以树结构的形式保存下来，
   以便以后的处理，如：

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
    其XMLTree(如下：
                               A
              ┌-----------|---------┐
               B               D           E
               |                 |
     ┌----┬--┬----┐   TEXT     
    TEXT   C  TEXT  TEXT 
               |
            TEXT

 为此定义如下数据结构：

  struct XML_TREE {
    char * tag_name;  /* 标识名 */
    char * att_list;  /* 属性  */
    int   lineno;     /* 标识所在的行号  */
    int  type;        /* 表示类型, PCDATA for TEXT;
                          ELEMENT for general TAG;
                          EMPTY for Empty Element Tag */

    struct List {     /* child list */
      struct XML_TREE * node;
      struct List * next;
    } * child_list;
  };

  typedef struct List List;

  typedef struct XML_TREE Xml_tree;

  相关操作如下：
  - Xml_tree * make_node(int type, char * tag,  char * att, int lineno);
     /* 建立一个树结点，其child_list = NULL */
  - List  * make_list( Xml_tree * node);
     /* 建立有一个元素(node)的链表；
  - add_child (List * parent, List * child);
     /* 将链表中的第一个元素的的child_list设为child */
  - void add_list ( List * header, List * tail);
     /* 链接两个List header和tail */
  - void  print_tree(int level, List * header);
     /* 对XML Tree遍历并格式化输出 xml文件 */

  相关的头文件和实现见xml_tree.h和xml_tree.c(已给出源码)

4/ 样本程序的运行:
    c:> xmlparse.exe < sample.xml | more (DOS)
    $ xmlparser < sample | more (linux)
5/ 程序清单
    Makefile		Linux Makefile
    tcmake.mak 		Turbo C Makefile
		        调用方式：
			1、C:>path=c:\tc;%path
			2、make -ftcmake.mak
    token.h		单词编码的宏定义
    xml_tree.h		xml tree数据结构的定义和
  			相关操作的头文件；
    xml_tree.c		xml_tree的实现(已提供源码)
    xml.l		XML词法分析flex源程序
    parser.c		递归下降分析器
			(需要完成！)
    xmlparser		linux样本程序执行文件(64位)
    xmlparse.exe	DOS样本程序执行文件
    sample.xml		测试文件
    REC-xml-20040204.xml XML规范，也可用来测试
			但是由于TC实模式对内存的限制
			在DOS下运行实会产生"Out
			of memory的错误，LINUX则没有
			相应的错误。
    可扩展标记语言(XML) 1.0.doc
			XML规范的汉语版
    readme.txt		本文件

    
6/ 设计任务
 
  a/ 完成递归下降分析程序List *tag_list(), 在正确分析了XML文件之后，
     返回其XML Tree List指针;

    (见源代码parser.c中的 /* 请完成！*/ 部分)
  
  b/ 作用提交到 mailto:hanfei.wang@gmail.com
     subject: 学号(2)
     内容: 仅附上parser.c！！
     ddl: 见群通知

7/ 参考源码： an open source html parser:
   HTML Tidy (https://github.com/htacg/tidy-html5)
   对start tag和end tag不平衡有特殊的处理。

hfwang

2021.09.15.
