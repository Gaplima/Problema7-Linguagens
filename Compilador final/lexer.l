%{
  #include <stdio.h>
  #include <string.h>
  #include "y.tab.h"
  
  extern int yyerror (char *msg);
%}

%option yylineno

%x COMMENT

%%

[ \t\r\n]             ; /* Ignora espacos, tabs e QUEBRAS DE LINHA DO WINDOWS (\r) */

"/*"                  { BEGIN(COMMENT); }
<COMMENT>"*/"         { BEGIN(INITIAL); }
<COMMENT>(.|\n)       ;

[0-9]+                { 
                        yylval.iValue = atoi(yytext);
                        return NUMBER;
                      }

"while"               { return(WHILE); }
"for"                 { return(FOR); }
"to"                  { return(TO); }
"begin"               { return(BLOCK_BEGIN); }
"end"                 { return(BLOCK_END); }
"do"                  { return(DO); }
"if"                  { return(IF); }
"then"                { return(THEN); }
"else"                { return(ELSE); }
"elif"                { return(ELIF); }
"goto"                { return(GOTO); }
":"                   { return(COLON); }

";"                   { return(SEMI); }
":="                  { return(ASSIGN); }
"and"                 { return(AND); }
"or"                  { return(OR); }

","                   { return(yytext[0]); }
"+"                   { return(yytext[0]); }
"-"                   { return(yytext[0]); }
"*"                   { return(yytext[0]); }
"/"                   { return(yytext[0]); }
"%"                   { return(yytext[0]); }
"^"                   { return(POWER); }

">="                  { return(GREATER_THAN_OR_EQUALS); }
"<="                  { return(LESS_THAN_OR_EQUALS); }
">"                   { return(GREATER_THAN); }
"<"                   { return(LESS_THAN); }
"=="                  { return(EQUALS); }
"="                   { return(yytext[0]); }

"int"                 { return(TYPE_INT); }
"arr[]"               { return(TYPE_ARRAY); }
"char"                { return(TYPE_CHAR); }
"string"              { return(TYPE_STRING); }
"float"               { return(TYPE_FLOAT); }

"["                   { return(yytext[0]); }
"]"                   { return(yytext[0]); }
"("                   { return(yytext[0]); }
")"                   { return(yytext[0]); }
"{"                   { return(yytext[0]); }
"}"                   { return(yytext[0]); }
"."                   { return(DOT); }

"return"              { return(RETURN); }
"echo"                { return(PRINT); }
"read"                { return(READ); }
"unit"                { return(UNIT); }

\"[^"\n]*\"           { 
                        yylval.sValue = strdup(yytext); 
                        return STRING_LITERAL; 
                      }

[0-9]+\.[0-9]+        { 
                        yylval.fValue = atof(yytext); 
                        return FLOAT_LITERAL; 
                      }

[a-zA-Z_][a-zA-Z0-9_]* { 
                        yylval.sValue = strdup(yytext); 
                        return ID; 
                      }

.                     { yyerror("Caractere invalido"); }

%%

#ifndef yywrap
   int yywrap (void) {return 1;}
#endif