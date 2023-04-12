# BeerAn编译器

## content：
flex & bison & llvm & c++14
如果没有安装，是不能直接运行的，可以尝试着进行安装后进行，另外，运行环境为Linux系统。

安装命令
```shell
   sudo apt-get install flex bison llvm gcc
```

## usage:
```shell
    git clone https://github.com/tc6-01/BeerAn-compiler.git
    cd BeerAn-compiler
    apt install flex bison llvm
    make
    ./BeerAn < BeerAn.bc > BeerAn.txt
    cat BeerAn.txt
```

## reference
[创建新语言](https://llvm.org/docs/tutorial/index.html)
[IRCode生成](https://llvm.org/doxygen/classllvm_1_1IRBuilder.html)


## documention
bc文法 这里定义了一个编程语言称作bc
编写BeerAn.bc程序，即简单的bc语言
该语言的语法规则与c基本类似，但是唯一不同的便是不用分号
当前只支持int，double两种类型。
程序样例如下：
 
 ![image](https://user-images.githubusercontent.com/78250618/231400458-8a4751ad-9577-44f1-b614-c774fb3a4bbf.png)

说明：本程序需要在BeerAn{}内部编写程序。
支持函数定义：例如int func(int a, int b),
另外要求函数必须要有返回值，且当前只支持返回一个值：return a + b

支持函数调用，可以在定义之后调用函数，如b = func(a, b)
支持if条件判断以及多层嵌套如：if(a>b){ if (a ==b){a = a + b}}
支持for循环以及多层嵌套：如for(int I; I <3 ; I + 1){for (int d=0; d > 3; d++){d = d +d}},另外for循环的初始化，条件判断，增量均需要填写
同时支持各种二元操作符，如：==，<=,>=,>,< &, +, - , *, /, % | , ^等等(不支持++，--操作)
支持变量定义: int a, double c
另外本语言在语句结束时不需要使用分号
程序结构描述（程序调用流程图）：
 ![image](https://user-images.githubusercontent.com/78250618/231401074-2fbd2e0b-fd6b-4fc0-8d3e-1a01f3c49cba.png)

bc语言词法分析文件（BeerAn.l）
1.	语言关键字 
BeerAn if else int double return for 
2.	下面是一些运算符和括号
+ - * / = > < >= <= % ^ & | == != { } ( ) 
3.	其他标记是通过正则表达式匹配得到的字符
标识符 [a-zA-Z_][a-zA-Z0-9_]*
浮点数 [0-9]+\.[0-9]*
整型数 [0-9]+
4.	定义行数line
int line = 0;
并在识别token之后进行输出
cout<<line<<"\t"<<"if"<<endl; return TOKEN(TIF);
5.	匹配换行符之后进行行数相加
 ![image](https://user-images.githubusercontent.com/78250618/231401119-5b88285b-db82-4755-bedc-efac931354f6.png)

6.	定义Savetoken，将识别的token转换为字符串并返回地址
7.	定义Token(t)，将识别token的字面量值设为本身
bc的语法
1.	program -> BEERAN TLBRACE statements TRBRACE
2.	statements -> statement | statements statement  
3.	statement-> var_decl | func_decl | expr| TRETURN expr| if_block | for_block
4.	expr ->	ident TEQUAL expr | ident  TLPAREN call_args TRPAREN| ident|numeric|expr comparison expr|expr TMOD expr| expr TMUL expr| expr TDIV expr| expr TPLUS expr| expr TMINUS expr| TLPAREN expr TRPAREN
5.	ident ->TIDENTIFIER
6.	numeric ->TINTEGER	| TDOUBLE
7.	def ->TYINT| TYDOUBLE 
8.	func_decl-> def ident TLPAREN func_decl_args TRPAREN block
9.	func_decl_args -> ε | var_decl| func_decl_args TCOMMA var_decl
10.	call_args ->ε| expr | call_args TCOMMA expr
11.	block-> TLBRACE statements TRBRACE| TLBRACE TRBRACE
12.	comparison-> TCEQ | TCNE | TCLT | TCLE | TCGT | TCGE| TAND | TOR | TXOR | TSHIFTL | TSHIFTR
13.	if_block->TIF expr block	| TIF expr block TELSE block | TIF expr block TELSE if_block
14.	for_block->TFOR TLPAREN expr TFENHAO expr TFENHAO expr TRPAREN block
下面是对以上每条文法规则，给出了相关语义的简短解释
1.	program -> BEERAN TLBRACE statements TRBRACE
2.	statements -> statement | statements statement  
3.	statement-> var_decl | func_decl | expr| TRETURN expr| if_block | for_block
识别BeerAn程序，并开始匹配多条语句，依次匹配变量声明，函数声明，返回表达式，if逻辑，for循环语句
4.	expr ->ident TEQUAL expr | ident  TLPAREN call_args TRPAREN| ident|numeric|expr comparison expr|expr TMOD expr| expr TMUL expr| expr TDIV expr| expr TPLUS expr| expr TMINUS expr| TLPAREN expr TRPAREN
5.	ident ->TIDENTIFIER
6.	numeric ->TINTEGER	| TDOUBLE
7.	def ->TYINT| TYDOUBLE 
8. comparison-> TCEQ | TCNE | TCLT | TCLE | TCGT | TCGE| TAND | TOR | TXOR | TSHIFTL | TSHIFTR
表达式分别匹配运算符（加减乘除，逻辑运算），变量名与数值
9.	func_decl-> def ident TLPAREN func_decl_args TRPAREN block
10.	func_decl_args -> ε | var_decl| func_decl_args TCOMMA var_decl
11.	call_args ->ε| expr | call_args TCOMMA expr
12.	block-> TLBRACE statements TRBRACE| TLBRACE TRBRACE
函数声明首先匹配返回类型，函数名，然后匹配参数，以及函数内部块儿
调用函数只需要匹配函数以及对应的参数即可。
程序块儿会匹配到多条语句
13.	if_block->TIF expr block	| TIF expr block TELSE block | TIF expr block TELSE if_block
14.	for_block->TFOR TLPAREN expr TFENHAO expr TFENHAO expr TRPAREN block
If循环语句可以添加else，也可以不添加else，另外在if内部可以再次添加if循环结构，同样在else内部也可以添加
bc 语言的语法树
	定义基类节点以及语义分析接口（中间代码生成）
 ![image](https://user-images.githubusercontent.com/78250618/231401265-f410a9c3-631c-4c35-a743-d057b91258db.png)

定义基类节点并以此为基础分别使用继承建立其他类型节点，如：
 ![image](https://user-images.githubusercontent.com/78250618/231401293-ca594922-b1b5-44b8-bc6c-57efe361fcc1.png)

下面对于算术操作，函数定义和程序块，for四类节点做出详细介绍
 ![image](https://user-images.githubusercontent.com/78250618/231401326-71ce5997-f1dc-4825-a59d-03365660043d.png)

定义类成员，操作符，左操作数，右操作数，其中做右操作数都是WangExpression节点，也可以调用这类节点的print函数，来实现做右操作数的打印。此外利用构造函数进行赋值，最重要的就是打印操作，其中m_prefix是’--’，m_delim是“:”用来打印语法树。另外最后的codeGen是用来生成中间代码的预留接口。
 ![image](https://user-images.githubusercontent.com/78250618/231401354-9281821e-8f54-46d3-8d5b-fe6f98bc8af4.png)

由于函数内部存放有返回类型，函数名，参数，函数内部语句，因此使用其他类节点，值得注意的是这里使用的是智能指针，可以使用同一个堆内存，减少了程序内存占用过高的风险。类似地，函数定义节点的打印函数同样是调用其内部成员类的print函数进行打印。同样，这里也为语义分析的中间代码生成保留了接口。
 ![image](https://user-images.githubusercontent.com/78250618/231401383-cafcde68-5ce1-497f-82ee-0b2c35669582.png)

程序块节点由于内部存放了多条语句，因此使用事先建立的存放多条语句的哈希表statementsList来存放。同样的，该类节点的print函数也会调用其他类的print函数，这里根据继承机制，print函数会根据语句的节点类型来实现。
 ![image](https://user-images.githubusercontent.com/78250618/231401410-180c6933-4733-4fbd-b3b1-29de6663a5a5.png)

不同于其他的类节点，if和for类节点都有条件判断部分，而for循环中则增加了初始化，迭代部分，因此需要将其存放起来。这里使用WangExpression节点存放三部分，使用WangBlock存放代码块儿，另外使用构造函数进行赋值，打印函数同样使用了继承。最后页保留了语义分析接口。
bc语言的语义分析（中间代码生成--IRCode）
在开始之前，首先设置符号表
 ![image](https://user-images.githubusercontent.com/78250618/231401451-4651a381-8ac7-4ced-bb63-a6fa8397b7da.png)

这里没有函数符号表的原因是在生成llvm中间代码的时候提供了一个符号表，在函数进行调用的时候直接返回生成的函数对象。如下图所示
声明时，创建函数对象，并自动添加进入函数符号表。
官方文档做法
 ![image](https://user-images.githubusercontent.com/78250618/231401474-7ece8046-e533-4781-8ac1-6db7a3880c93.png)

声明时创建函数对象并自动添加至函数表中
 ![image](https://user-images.githubusercontent.com/78250618/231401507-8cfa9be7-ad0a-48d5-a20c-19d61fbc21b5.png)

调用时，查找函数表。
 ![image](https://user-images.githubusercontent.com/78250618/231401527-51a73aff-9d98-4353-b7bd-a2c8e6fe5eb7.png)

另外，将程序block使用栈进行存储
 ![image](https://user-images.githubusercontent.com/78250618/231401541-89270a39-44ca-4724-a6b0-ab0ba6ca5aef.png)

之后进行语义分析方法的实现，如查找符号表等，具体如下图所示
 ![image](https://user-images.githubusercontent.com/78250618/231401569-49e35778-f024-4827-b8bc-405f50571e98.png)

另外，实现上述基础操作后，以此实现在抽象语法树定义的各类节点的中间代码生成。这里以变量声明节点的codeGen来进行说明
 ![image](https://user-images.githubusercontent.com/78250618/231401596-2b5495a2-2524-4dc6-9f24-481935bc33ce.png)

首先将语法分析中的返回值转换为llvm对应的类型，然后按照llvm文档中提供的api建立对应的IR，即调用分配空间指令，并返回该指令的地址，并将其地址存放进符号表中。类型表与符号表分别进行存储。之后将后续的部分将给对应的节点进行代码生成
bc makefile程序进行编译
 ![image](https://user-images.githubusercontent.com/78250618/231400929-4467c39a-9e81-497b-96f4-225f6f5a7f42.png)

Make BeerAn 进行词法语法的编译以及中间代码生成，并生成可执行程序，同时将生成的.o文件删除。
Make clean 删除所有生成文件，得到初始文件。
编译运行：
Make 得到可执行文件 BeerAn
 ![image](https://user-images.githubusercontent.com/78250618/231400893-6c70c228-2d8e-4aa1-a022-71229741463a.png)

接着进行运行输入源程序进行执行
./BeerAn <BeerAn.bc > BeerAn.txt（其中BeerAn.bc程序在最开始已经给出）                    
![image](https://user-images.githubusercontent.com/78250618/231400742-0be2e9df-74dd-4d92-9138-358c16f4e156.png)


