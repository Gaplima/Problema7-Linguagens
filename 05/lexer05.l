%{
  #include <stdio.h>
  #include "y.tab.h"
  
  extern int yyerror (char *msg);  
%}

%%

[ \t\n]               ;
[0-9]+           		  {yylval.iValue = atoi(yytext); 
                       return NUMBER;}
"function"            {return(FUNCTION);}
"procedure"           {return(PROCEDURE);}
"begin"               {return(BEGIN_TOKEN);}
"end"                 {return(END_TOKEN);}
"("                   {return(yytext[0]);}
")"                   {return(yytext[0]);}
":"                   {return(yytext[0]);}
","                   {return(yytext[0]);}
";"                   {return(yytext[0]);}
"while"      			    {return(WHILE);}
"do"      				    {return(DO);}
"if"      				    {return(IF);}
"then"      			    {return(THEN);}
"else"      			    {return(ELSE);}
":="					        {return ASSIGN;}
"int"                 {yylval.sValue = strdup("int"); 
                       return TYPE;}
"float"               {yylval.sValue = strdup("float"); 
                       return TYPE;}
[a-zA-Z][a-zA-Z0-9]*	{yylval.sValue = strdup(yytext); 
                       return ID;}
.                     {yyerror("invalid character");}

%%

#ifndef yywrap
   int yywrap (void) {return 1;}
#endif