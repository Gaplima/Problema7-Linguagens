%{
  #include <stdio.h>
  #include "y.tab.h"
  
  extern int yyerror (char *msg);  

  int recuo_anterior = 0, recuo_atual = 0;
%}

%%

[ \t]+                {}
[\n]+                 { if (recuo_anterior > 0){
                          recuo_anterior = 0;
                          return BACK;
                        }
                      } 
[\n]+[ ]+             { //quantos espaços no recuo, ignorando quebras de linha?
                        recuo_atual = strlen(yytext) - strcspn(yytext, " ");
                        if (recuo_atual > recuo_anterior)
                          recuo_anterior = recuo_atual;
                        else if (recuo_atual < recuo_anterior) {
                              recuo_anterior = recuo_atual;
                              return BACK;
                             }
                      }
"while"      			    {return(WHILE);}
":="					        {return ASSIGN;}
[a-zA-Z][a-zA-Z0-9]*	{yylval.sValue = strdup(yytext); 
                       return ID;}
.                     {yyerror("invalid character");}

%%

#ifndef yywrap
   int yywrap (void) {return 1;}
#endif