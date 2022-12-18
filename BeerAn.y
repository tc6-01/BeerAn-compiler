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
	WangStatement* statement;
	WangIdentifier* ident;
	WangVarDel* var_Del;
	std::vector<shared_ptr<WangVarDel>>* varvec;
	std::vector<shared_ptr<WangExpression>>* exprvec;
	std::string* string;
	int token;
}

// 定义终结符类型
// 定义变量与数值类型
%token <string> TIDENTIFIER TINTEGER TDOUBLE BEERAN
// 定义返回值类型
%token <string> TYINT TYDOUBLE  
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
%type <block> program statements block
%type <statement> statement var_decl func_decl if_block for_block
%type <token> comparison

// 定义运算优先级
%left TPLUS TMINUS
%left TMUL TDIV TMOD

%start program

%%
/*程序入口*/
program : BEERAN TLBRACE statements TRBRACE{ programBlock = $3; }
				;

/*包含语句的基本块儿*/
statements : statement { $$ = new WangBlock(); $$->statements->push_back(shared_ptr<WangStatement>($1)); }
			| statements statement { $1->statements->push_back(shared_ptr<WangStatement>($2)); }
			;

/*语句*/
statement : var_decl | func_decl 
		 | expr { $$ = new WangExpressionStatement(shared_ptr<WangExpression>($1)); }
		 | TRETURN expr { $$ = new WangReturn(shared_ptr<WangExpression>($2)); }
		 | if_block
		 | for_block
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

/*函数名或变量名*/
ident : TIDENTIFIER { $$ = new WangIdentifier(*$1); delete $1; }
			;

/*数值*/
numeric : TINTEGER { $$ = new WangInteger(atol($1->c_str())); }
			| TDOUBLE { $$ = new WangDouble(atof($1->c_str())); }
			;

/*类型定义*/
def : TYINT { $$ = new WangIdentifier(*$1); delete $1; }
					| TYDOUBLE { $$ = new WangIdentifier(*$1); delete $1; }

/*变量与函数声明*/
var_decl : def ident /*int i*/{ $$ = new WangVarDel(shared_ptr<WangIdentifier>($1), shared_ptr<WangIdentifier>($2), nullptr); }
				 | def ident TEQUAL expr /*int i = 3*/{ $$ = new WangVarDel(shared_ptr<WangIdentifier>($1), shared_ptr<WangIdentifier>($2), shared_ptr<WangExpression>($4)); }
				 ;

/*函数定义*/
func_decl : def ident TLPAREN func_decl_args TRPAREN block
				{ $$ = new WangFuncDel(shared_ptr<WangIdentifier>($1), shared_ptr<WangIdentifier>($2), shared_ptr<VariableList>($4), shared_ptr<WangBlock>($6));  }

/*函数参数*/
func_decl_args :              { $$ = new VariableList(); }
							 | var_decl { $$ = new VariableList(); $$->push_back(shared_ptr<WangVarDel>($<var_Del>1)); }
							 | func_decl_args TCOMMA var_decl { $1->push_back(shared_ptr<WangVarDel>($<var_Del>3)); }
							 ;

/*调用函数参数*/
call_args :         { $$ = new ExpressionList(); }
					| expr { $$ = new ExpressionList(); $$->push_back(shared_ptr<WangExpression>($1)); }
					| call_args TCOMMA expr { $1->push_back(shared_ptr<WangExpression>($3)); }

/*程序块儿*/
block : TLBRACE statements TRBRACE { $$ = $2; }
			| TLBRACE TRBRACE { $$ = new WangBlock(); }
			;



/*运算符*/
comparison : TCEQ | TCNE | TCLT | TCLE | TCGT | TCGE
				 | TAND | TOR | TXOR | TSHIFTL | TSHIFTR
			;

/*if控制语句*/
if_block : TIF expr block { $$ = new WangIf(shared_ptr<WangExpression>($2), shared_ptr<WangBlock>($3)); }
		| TIF expr block TELSE block { $$ = new WangIf(shared_ptr<WangExpression>($2), shared_ptr<WangBlock>($3), shared_ptr<WangBlock>($5)); }
		| TIF expr block TELSE if_block { 
			auto blk = new WangBlock(); 
			blk->statements->push_back(shared_ptr<WangStatement>($5)); 
			$$ = new WangIf(shared_ptr<WangExpression>($2), shared_ptr<WangBlock>($3), shared_ptr<WangBlock>(blk)); 
		}

/*for控制语句*/
for_block : TFOR TLPAREN expr TFENHAO expr TFENHAO expr TRPAREN block { $$ = new WangFOR(shared_ptr<WangBlock>($9), shared_ptr<WangExpression>($3), shared_ptr<WangExpression>($5), shared_ptr<WangExpression>($7)); }
%%