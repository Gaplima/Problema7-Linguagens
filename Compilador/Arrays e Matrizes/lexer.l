%{
  #include <stdio.h>
  #include "y.tab.h"
  
  extern int yyerror (char *msg);
%}

/* Declara estado exclusivo para comentários */
%x COMMENT

%%

[ \t\n]               ; /* Ignora espaços e quebras de linha */

"/*"                  { BEGIN(COMMENT); } /* Entra no modo comentário */
<COMMENT>"*/"         { BEGIN(INITIAL); } /* Sai do modo comentário */
<COMMENT>(.|\n)       ; /* Ignora qualquer coisa dentro do comentário */

[0-9]+           	  {yylval.iValue = atoi(yytext); return NUMBER;}

"while"      		  {return(WHILE);}
"for"             {return(FOR);}
"to"              {return(TO);}
"begin"      		  {return(BLOCK_BEGIN);}
"end"      			  {return(BLOCK_END);}
"do"      			  {return(DO);}
"if"      			  {return(IF);}
"then"      		  {return(THEN);}
"else"      		  {return(ELSE);}
"elif"                {return(ELIF);}

";"                   {return(SEMI);}
":="				  {return ASSIGN;}
"and"                 {return(AND);}
"or"                  {return(OR);}

","                   {return(yytext[0]);}
"+"                   {return(yytext[0]);}
"-"                   {return(yytext[0]);}
"*"                   {return(yytext[0]);}
"/"                   {return(yytext[0]);} 
"%"                   {return(yytext[0]);}
"^"                   {return(POWER);}

"++"                  {return(OP_ADD_ONE);}
"--"                  {return(OP_SUB_ONE);}
"sizeof"              {return(OP_SIZEOF);}

">="                  {return(GREATER_THAN_OR_EQUALS);}
"<="                  {return(LESS_THAN_OR_EQUALS);}
">"                   {return(GREATER_THAN);}
"<"                   {return(LESS_THAN);}
"=="                  {return(EQUALS);}
"="                   {return(yytext[0]);}

"int"                 {return(TYPE_INT);}
"arr[]"               {return(TYPE_ARRAY);}
"char"                {return(TYPE_CHAR);}
"string"              {return(TYPE_STRING);}
"float"               {return(TYPE_FLOAT);}

"["                   {return(yytext[0]);}
"]"                   {return(yytext[0]);}
"("                   {return(yytext[0]);}
")"                   {return(yytext[0]);}
"{"                   {return(yytext[0]);}
"}"                   {return(yytext[0]);}

[a-zA-Z][a-zA-Z0-9]* {yylval.sValue = strdup(yytext); return ID;}

.                     {yyerror("Caractere invalido");}

%%

#ifndef yywrap
   int yywrap (void) {return 1;}
#endif