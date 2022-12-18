%{
#include <iostream>
#include <string>
#include "ASTNodes.h"
#include "BeerAnSytnax.hpp"
#define SAVE_TOKEN yylval.string = new string(yytext)
#define TOKEN(t) ( yylval.token = t)
using namespace std;
static FILE* yyparse_file_ptr;
int line = 0;
%}

%option noyywrap

%%
"#".*                   ;
[ \t\r]  				;
[\n]                    line++;
"if"                    cout<<line<<"\t"<<"if"<<endl; return TOKEN(TIF);
"else"                  cout<<line<<"\t"<<"else"<<endl; return TOKEN(TELSE);
"return"                cout<<line<<"\t"<<"return"<<endl; return TOKEN(TRETURN);
"for"                   cout<<line<<"\t"<<"for"<<endl; return TOKEN(TFOR);
"int"                   SAVE_TOKEN; cout<<line<<"\t"<<"int"<<endl;  return TYINT;
"double"                SAVE_TOKEN; cout<<line<<"\t"<<"double"<<endl; return TYDOUBLE;
"BeerAn"                SAVE_TOKEN; cout<<line<<"\t"<<"BEERAN"<<endl; return BEERAN;
[a-zA-Z_][a-zA-Z0-9_]*	SAVE_TOKEN; cout<<line<<"\t"<<"变量名"<<endl; return TIDENTIFIER;
[0-9]+\.[0-9]*			SAVE_TOKEN; cout<<line<<"\t"<<"浮点数"<<endl; return TDOUBLE;
[0-9]+  				SAVE_TOKEN; cout<<line<<"\t"<<"整数"<<endl; return TINTEGER;
"="						cout<<line<<"\t"<<"="<<endl; return TOKEN(TEQUAL);
"=="					cout<<line<<"\t"<<"=="<<endl; return TOKEN(TCEQ);
"!="                    cout<<line<<"\t"<<"!="<<endl; return TOKEN(TCNE);
"<"                     cout<<line<<"\t"<<"<"<<endl; return TOKEN(TCLT);
"<="                    cout<<line<<"\t"<<"<="<<endl; return TOKEN(TCLE);
">"                     cout<<line<<"\t"<<">"<<endl; return TOKEN(TCGT);
">="                    cout<<line<<"\t"<<">="<<endl; return TOKEN(TCGE);
"("                     cout<<line<<"\t"<<"("<<endl; return TOKEN(TLPAREN);
")"                     cout<<line<<"\t"<<")"<<endl; return TOKEN(TRPAREN);
"{"                     cout<<line<<"\t"<<"{"<<endl; return TOKEN(TLBRACE);
"}"                     cout<<line<<"\t"<<"}"<<endl; return TOKEN(TRBRACE);
","                     cout<<line<<"\t"<<","<<endl; return TOKEN(TCOMMA);
"+"                     cout<<line<<"\t"<<"+"<<endl; return TOKEN(TPLUS);
"-"                     cout<<line<<"\t"<<"-"<<endl; return TOKEN(TMINUS);
"*"                     cout<<line<<"\t"<<"*"<<endl; return TOKEN(TMUL);
"/"                     cout<<line<<"\t"<<"/"<<endl; return TOKEN(TDIV);
"&"                     cout<<line<<"\t"<<"&"<<endl; return TOKEN(TAND);
"|"                     cout<<line<<"\t"<<"|"<<endl; return TOKEN(TOR);
"^"                     cout<<line<<"\t"<<"^"<<endl; return TOKEN(TXOR);
"%"                     cout<<line<<"\t"<<"%"<<endl; return TOKEN(TMOD);
">>"                    cout<<line<<"\t"<<">>"<<endl; return TOKEN(TSHIFTR);
"<<"                    cout<<line<<"\t"<<"<<"<<endl; return TOKEN(TSHIFTL);
";"                     cout<<line<<"\t"<<";"<<endl; return TOKEN(TFENHAO);
.						cout<<line<<"\t"<<"Unknown token: "<<yytext<<endl; yyterminate();
%%

