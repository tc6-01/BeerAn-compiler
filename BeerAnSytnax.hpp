/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_YY_BEERANSYTNAX_HPP_INCLUDED
# define YY_YY_BEERANSYTNAX_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    TIDENTIFIER = 258,
    TINTEGER = 259,
    TDOUBLE = 260,
    BEERAN = 261,
    TYINT = 262,
    TYDOUBLE = 263,
    TYFLOAT = 264,
    TYCHAR = 265,
    TYBOOL = 266,
    TYVOID = 267,
    TYSTRING = 268,
    TLPAREN = 269,
    TRPAREN = 270,
    TLBRACE = 271,
    TRBRACE = 272,
    TCOMMA = 273,
    TFENHAO = 274,
    TPLUS = 275,
    TMINUS = 276,
    TMUL = 277,
    TDIV = 278,
    TAND = 279,
    TOR = 280,
    TXOR = 281,
    TMOD = 282,
    TNOT = 283,
    TSHIFTL = 284,
    TSHIFTR = 285,
    TCEQ = 286,
    TCNE = 287,
    TCLT = 288,
    TCLE = 289,
    TCGT = 290,
    TCGE = 291,
    TEQUAL = 292,
    TIF = 293,
    TELSE = 294,
    TFOR = 295,
    TRETURN = 296
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 15 "BeerAn.y"

	WangBlock* block;
	WangExpression* expr;
	WangStatement* stmt;
	WangIdentifier* ident;
	WangVarDel* var_decl;
	std::vector<shared_ptr<WangVarDel>>* varvec;
	std::vector<shared_ptr<WangExpression>>* exprvec;
	std::string* string;
	int token;

#line 111 "BeerAnSytnax.hpp"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_BEERANSYTNAX_HPP_INCLUDED  */
