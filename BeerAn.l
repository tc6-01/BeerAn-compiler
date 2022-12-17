%{
#include <stdio.h>
#include <string>
#include "ASTNodes.h"
#include "BeerAnSytnax.hpp"
#define SAVE_TOKEN yylval.string = new string(yytext)
#define TOKEN(t) ( yylval.token = t)
static FILE* yyparse_file_ptr;
%}

%option noyywrap

%%
"#".*                   ;
[ \t\r\n]				;
"if"                    puts("TIF"); return TOKEN(TIF);
"else"                  puts("TELSE"); return TOKEN(TELSE);
"return"                puts("TRETURN"); return TOKEN(TRETURN);
"for"                   puts("TFOR"); return TOKEN(TFOR);
"int"                   SAVE_TOKEN; puts("TYINT");  return TYINT;
"double"                SAVE_TOKEN; puts("TYDOUBLE"); return TYDOUBLE;
"float"                 SAVE_TOKEN; puts("TYFLOAT"); return TYFLOAT;
"char"                  SAVE_TOKEN; puts("TYCHAR"); return TYCHAR;
"BeerAn"                SAVE_TOKEN; puts("BEERAN"); return BEERAN;
"bool"                  SAVE_TOKEN; puts("TYBOOL"); return TYBOOL;
"string"                SAVE_TOKEN; puts("TYSTRING"); return TYSTRING;
"void"                  SAVE_TOKEN; puts("TYVOID"); return TYVOID;
[a-zA-Z_][a-zA-Z0-9_]*	SAVE_TOKEN; puts("TIDENTIFIER"); return TIDENTIFIER;
[0-9]+\.[0-9]*			SAVE_TOKEN; puts("TDOUBLE"); return TDOUBLE;
[0-9]+  				SAVE_TOKEN; puts("TINTEGER"); return TINTEGER;
"="						puts("TEQUAL"); return TOKEN(TEQUAL);
"=="					puts("TCEQ"); return TOKEN(TCEQ);
"!="                    puts("TCNE"); return TOKEN(TCNE);
"<"                     puts("TCLT"); return TOKEN(TCLT);
"<="                    puts("TCLE"); return TOKEN(TCLE);
">"                     puts("TCGT"); return TOKEN(TCGT);
">="                    puts("TCGE"); return TOKEN(TCGE);
"("                     puts("TLPAREN"); return TOKEN(TLPAREN);
")"                     puts("TRPAREN"); return TOKEN(TRPAREN);
"{"                     puts("TLBRACE"); return TOKEN(TLBRACE);
"}"                     puts("TRBRACE"); return TOKEN(TRBRACE);
","                     puts("TCOMMA"); return TOKEN(TCOMMA);
"+"                     puts("TPLUS"); return TOKEN(TPLUS);
"-"                     puts("TMINUS"); return TOKEN(TMINUS);
"*"                     puts("TMUL"); return TOKEN(TMUL);
"/"                     puts("TDIV"); return TOKEN(TDIV);
"&"                     puts("TAND"); return TOKEN(TAND);
"|"                     puts("TOR"); return TOKEN(TOR);
"^"                     puts("TXOR"); return TOKEN(TXOR);
"%"                     puts("TMOD"); return TOKEN(TMOD);
">>"                    puts("TSHIFTR"); return TOKEN(TSHIFTR);
"<<"                    puts("TSHIFTL"); return TOKEN(TSHIFTL);
";"                     puts("TFENHAO"); return TOKEN(TFENHAO);
.						printf("Unknown token:%s\n", yytext); yyterminate();
%%
