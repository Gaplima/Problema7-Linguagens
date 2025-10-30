%{
  #include <stdio.h>
  #include "y.tab.h"
  
  extern int yyerror (char *msg);  
%}

%%

[ \t]                 ;
\n                    {yylineno++;}
[0-9]+           		  {yylval.iValue = atoi(yytext); 
                       return NUMBER;}
"["                   {return(yytext[0]);}
"]"                   {return(yytext[0]);}                      
","                   {return(yytext[0]);}                      
":"                   {return(yytext[0]);}                      
"int"                 {return(TYPE);}
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
.                     {yyerror("invalid character");}

%%

#ifndef yywrap
   int yywrap (void) {return 1;}
#endif