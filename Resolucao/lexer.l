%{
  #include <stdio.h>
  #include "y.tab.h"
  
  extern int yyerror (char *msg);  
%}

%%

[ \t\n]               ;
[0-9]+           		  {yylval.iValue = atoi(yytext); 
                       return NUMBER;}
"while"      			    {return(WHILE);}
"begin"      			    {return(BLOCK_BEGIN);}
"end"      				    {return(BLOCK_END);}
"do"      				    {return(DO);}
"if"      				    {return(IF);}
"then"      			    {return(THEN);}
"else"      			    {return(ELSE);}
";"      				      {return(SEMI);}
":="					        {return ASSIGN;}
[a-zA-Z][a-zA-Z0-9]*	{yylval.sValue = strdup(yytext); 
                       return ID;}
"elif"                {return(ELSE IF);}
"and"                 {return(AND);}
"or"                  {return(OR);}
";"                   {return(yytext[0]);}
","                   {return(yytext[0]);}
"%d"                  {return(VALUE);}

"+"                   {return(yytext[0]);}
"-"                   {return(yytext[0]);}
"*"                   {return(yytext[0]);}
"/"                   {return(yytext[0]);}
"%"                   {return(yytext[0]);}
"++"                  {return(OP_ADD_ONE);}
"--"                  {return(OP_SUB_ONE);}
"sizeof"              {return(OP_SIZEOF);}
">="                  {return(GREATER_THAN_OR_EQUALS);}
"<="                  {return(LESS_THAN_OR_EQUALS);}
">"                   {return(yytext[0]);}
"<"                   {return(yytext[0]);}
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
"//".*                {return(COMENTARIO: %s\n", yytext);}

.                     {yyerror("errrrrrooooooooou");}

%%

#ifndef yywrap
   int yywrap (void) {return 1;}
#endif