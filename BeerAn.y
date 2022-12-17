%{
	#include "ASTNodes.h"
	#include <stdio.h>
	// 定义抽象语法树根节点
	WangBlock* programBlock;
	extern int yylex();
	void yyerror(const char* s)
	{
		printf("Error: %s\n", s);
	}
%}

/*返回节点类型定义*/
%union
{
	WangBlock* block;
	WangExpression* expr;
	WangStatement* stmt;
	WangIdentifier* ident;
	WangVarDel* var_decl;
	std::vector<shared_ptr<WangVarDel>>* varvec;
	std::vector<shared_ptr<WangExpression>>* exprvec;
	std::string* string;
	int token;
}

// 定义终结符类型
// 定义变量与数值类型
%token <string> TIDENTIFIER TINTEGER TDOUBLE BEERAN
// 定义返回值类型
%token <string> TYINT TYDOUBLE TYFLOAT TYCHAR TYBOOL TYVOID TYSTRING 
// 定义括号，逗号与分号类型
%token <token> TLPAREN TRPAREN TLBRACE TRBRACE TCOMMA TFENHAO
// 定义运算符类型
%token <token> TPLUS TMINUS TMUL TDIV TAND TOR TXOR TMOD TNOT TSHIFTL TSHIFTR TCEQ TCNE TCLT TCLE TCGT TCGE TEQUAL
// 定义控制关键字类型
%token <token> TIF TELSE TFOR TRETURN 

// 定义非终结符类型
%type <ident> ident def
%type <expr> numeric expr 
%type <varvec> func_decl_args 
%type <exprvec> call_args
%type <block> program stmts block
%type <stmt> stmt var_decl func_decl if_stmt for_stmt
%type <token> comparison

// 定义运算优先级
%left TPLUS TMINUS
%left TMUL TDIV TMOD

%start program

%%
/*程序入口*/
program : BEERAN TLBRACE stmts TRBRACE{ programBlock = $3; }
				;

/*包含语句的基本块儿*/
stmts : stmt { $$ = new WangBlock(); $$->statements->push_back(shared_ptr<WangStatement>($1)); }
			| stmts stmt { $1->statements->push_back(shared_ptr<WangStatement>($2)); }
			;

/*语句*/
stmt : var_decl | func_decl 
		 | expr { $$ = new WangExpressionStatement(shared_ptr<WangExpression>($1)); }
		 | TRETURN expr { $$ = new WangReturn(shared_ptr<WangExpression>($2)); }
		 | if_stmt
		 | for_stmt
		 ;

/*程序块儿*/
block : TLBRACE stmts TRBRACE { $$ = $2; }
			| TLBRACE TRBRACE { $$ = new WangBlock(); }
			;

/*类型定义*/
def : TYINT { $$ = new WangIdentifier(*$1); $$->isType = true;  delete $1; }
					| TYDOUBLE { $$ = new WangIdentifier(*$1); $$->isType = true; delete $1; }
					| TYFLOAT { $$ = new WangIdentifier(*$1); $$->isType = true; delete $1; }
					| TYCHAR { $$ = new WangIdentifier(*$1); $$->isType = true; delete $1; }
					| TYBOOL { $$ = new WangIdentifier(*$1); $$->isType = true; delete $1; }
					| TYVOID { $$ = new WangIdentifier(*$1); $$->isType = true; delete $1; }
					| TYSTRING { $$ = new WangIdentifier(*$1); $$->isType = true; delete $1; }

/*变量与函数声明*/
var_decl : def ident /*int i*/{ $$ = new WangVarDel(shared_ptr<WangIdentifier>($1), shared_ptr<WangIdentifier>($2), nullptr); }
				 | def ident TEQUAL expr /*int i = 3*/{ $$ = new WangVarDel(shared_ptr<WangIdentifier>($1), shared_ptr<WangIdentifier>($2), shared_ptr<WangExpression>($4)); }
				 ;

/*函数定义*/
func_decl : def ident TLPAREN func_decl_args TRPAREN block
				{ $$ = new WangFuncDel(shared_ptr<WangIdentifier>($1), shared_ptr<WangIdentifier>($2), shared_ptr<VariableList>($4), shared_ptr<WangBlock>($6));  }

/*函数参数*/
func_decl_args : /* blank */ { $$ = new VariableList(); }
							 | var_decl { $$ = new VariableList(); $$->push_back(shared_ptr<WangVarDel>($<var_decl>1)); }
							 | func_decl_args TCOMMA var_decl { $1->push_back(shared_ptr<WangVarDel>($<var_decl>3)); }
							 ;

/*函数名或变量名*/
ident : TIDENTIFIER { $$ = new WangIdentifier(*$1); delete $1; }
			;

/*数值*/
numeric : TINTEGER { $$ = new WangInteger(atol($1->c_str())); }
			| TDOUBLE { $$ = new WangDouble(atof($1->c_str())); }
			;

/*算术操作与赋值语句*/
expr : 	ident TEQUAL expr /*i = 1*/ { $$ = new WangAssignment(shared_ptr<WangIdentifier>($1), shared_ptr<WangExpression>($3));  }
		 | ident TLPAREN call_args TRPAREN /*func(1, 2)*/{ $$ = new WangCallFunc(shared_ptr<WangIdentifier>($1), shared_ptr<ExpressionList>($3)); }
		 | ident /* i */{ $<ident>$ = $1; }
		 | numeric /* 3 */
		 | expr comparison expr { $$ = new WangBinOp(shared_ptr<WangExpression>($1), $2, shared_ptr<WangExpression>($3)); }
		 | expr TMOD expr { $$ = new WangBinOp(shared_ptr<WangExpression>($1), $2, shared_ptr<WangExpression>($3)); }
		 | expr TMUL expr { $$ = new WangBinOp(shared_ptr<WangExpression>($1), $2, shared_ptr<WangExpression>($3)); }
		 | expr TDIV expr { $$ = new WangBinOp(shared_ptr<WangExpression>($1), $2, shared_ptr<WangExpression>($3)); }
		 | expr TPLUS expr { $$ = new WangBinOp(shared_ptr<WangExpression>($1), $2, shared_ptr<WangExpression>($3)); }
		 | expr TMINUS expr { $$ = new WangBinOp(shared_ptr<WangExpression>($1), $2, shared_ptr<WangExpression>($3)); }
		 | TLPAREN expr TRPAREN { $$ = $2; }
		 ;

/*调用函数参数*/
call_args : /* blank */ { $$ = new ExpressionList(); }
					| expr { $$ = new ExpressionList(); $$->push_back(shared_ptr<WangExpression>($1)); }
					| call_args TCOMMA expr { $1->push_back(shared_ptr<WangExpression>($3)); }

/*运算符*/
comparison : TCEQ | TCNE | TCLT | TCLE | TCGT | TCGE
				 | TAND | TOR | TXOR | TSHIFTL | TSHIFTR
			;

/*if控制语句*/
if_stmt : TIF expr block { $$ = new WangIf(shared_ptr<WangExpression>($2), shared_ptr<WangBlock>($3)); }
		| TIF expr block TELSE block { $$ = new WangIf(shared_ptr<WangExpression>($2), shared_ptr<WangBlock>($3), shared_ptr<WangBlock>($5)); }
		| TIF expr block TELSE if_stmt { 
			auto blk = new WangBlock(); 
			blk->statements->push_back(shared_ptr<WangStatement>($5)); 
			$$ = new WangIf(shared_ptr<WangExpression>($2), shared_ptr<WangBlock>($3), shared_ptr<WangBlock>(blk)); 
		}

/*for控制语句*/
for_stmt : TFOR TLPAREN expr TFENHAO expr TFENHAO expr TRPAREN block { $$ = new WangFOR(shared_ptr<WangBlock>($9), shared_ptr<WangExpression>($3), shared_ptr<WangExpression>($5), shared_ptr<WangExpression>($7)); }
%%