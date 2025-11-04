/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
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
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_Y_TAB_H_INCLUDED
# define YY_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    NUMBER = 258,                  /* NUMBER  */
    ID = 259,                      /* ID  */
    WHILE = 260,                   /* WHILE  */
    DO = 261,                      /* DO  */
    IF = 262,                      /* IF  */
    THEN = 263,                    /* THEN  */
    ELSE = 264,                    /* ELSE  */
    ELIF = 265,                    /* ELIF  */
    BLOCK_BEGIN = 266,             /* BLOCK_BEGIN  */
    BLOCK_END = 267,               /* BLOCK_END  */
    ASSIGN = 268,                  /* ASSIGN  */
    SEMI = 269,                    /* SEMI  */
    VALUE = 270,                   /* VALUE  */
    AND = 271,                     /* AND  */
    OR = 272,                      /* OR  */
    OP_ADD_ONE = 273,              /* OP_ADD_ONE  */
    OP_SUB_ONE = 274,              /* OP_SUB_ONE  */
    OP_SIZEOF = 275,               /* OP_SIZEOF  */
    GREATER_THAN_OR_EQUALS = 276,  /* GREATER_THAN_OR_EQUALS  */
    LESS_THAN_OR_EQUALS = 277,     /* LESS_THAN_OR_EQUALS  */
    EQUALS = 278,                  /* EQUALS  */
    TYPE_INT = 279,                /* TYPE_INT  */
    TYPE_ARRAY = 280,              /* TYPE_ARRAY  */
    TYPE_CHAR = 281,               /* TYPE_CHAR  */
    TYPE_STRING = 282,             /* TYPE_STRING  */
    TYPE_FLOAT = 283               /* TYPE_FLOAT  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 11 "parser.y"

    int iValue;
    char* sValue;

#line 97 "y.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
