%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int yylex(void);
int yyerror(char *s);
extern int yylineno;
extern char * yytext;

%}

%union {
	int    iValue; 	/* integer value */
	char   cValue; 	/* char value */
	char * sValue;  /* string value */
	};

%token <sValue> ID
%token <sValue> TYPE
%token <iValue> NUMBER
%token FUNCTION PROCEDURE BEGIN_TOKEN END_TOKEN WHILE DO IF THEN ELSE ASSIGN 

%type <sValue> corpo
%type <sValue> procedimento
%type <sValue> funcao
%type <sValue> subp
%type <sValue> subps args args_aux ids ids_aux

%start programa

%%
programa : subps corpo {printf("%s%s\n", $1, $2);
                        free($1);
                        free($2);} 
	     ;

subps :             {$$ = strdup("");} 
       | subp subps {int n1 = strlen($1);
                     int n2 = strlen($2);
                     char * s = malloc(sizeof(char)*(n1+n2+2));
                     sprintf(s, "%s\n%s", $1, $2);
                     free($1);
                     free($2);
                     $$ = s;} 
      ;

subp : funcao       {$$ = $1;} 
     | procedimento {$$ = $1;} 
     ;

funcao : FUNCTION ID '(' args ')' ':' TYPE corpo {int n1 = strlen($2);
                                                  int n2 = strlen($4);
                                                  int n3 = strlen($7);
                                                  int n4 = strlen($8);
                                                  char * s = malloc(sizeof(char)*(n1+n2+n3+n4+16));
                                                  sprintf(s, "FUNCTION %s(%s) : %s %s", $2, $4, $7, $8);
                                                  free($2);
                                                  free($4);
                                                  free($7);
                                                  free($8);
                                                  $$ = s;}  
       ;

procedimento : PROCEDURE ID '(' args ')' corpo {int n1 = strlen($2);
                                                int n2 = strlen($4);
                                                int n3 = strlen($6);
                                                char * s = malloc(sizeof(char)*(n1+n2+n3+14));
                                                sprintf(s, "PROCEDURE %s() %s", $2, $6);
                                                free($2);
                                                free($6);
                                                $$ = s;} 
             ;

args :          {$$ = strdup("");}
     | args_aux {$$ = $1;}
     ;

args_aux : TYPE ids               {int n1 = strlen($1);
                                   int n2 = strlen($2);
                                   char * s = malloc(sizeof(char) * (n1+n2+2));
                                   sprintf(s,"%s %s", $1, $2);
                                   free($1);
                                   free($2);
                                   $$ = s;}
         | TYPE ids ';' args_aux {int n1 = strlen($1);
                                  int n2 = strlen($2);
                                  int n3 = strlen($4);
                                  char * s = malloc(sizeof(char) * (n1+n2+n3+4));
                                  sprintf(s,"%s %s; %s", $1, $2, $4);
                                  free($1);
                                  free($2);
                                  free($4);
                                  $$ = s;}
         ;                  

ids :         {$$ = strdup("");}
    | ids_aux {$$ = $1;}
    ;

ids_aux : ID             {$$ = $1;}
        | ID ',' ids_aux {int n1 = strlen($1);
                          int n2 = strlen($3);
                          char * s = malloc(sizeof(char) * (n1+n2+3));
                          sprintf(s,"%s, %s", $1, $3);
                          free($1);
                          free($3);
                          $$ = s;}
        ;            

corpo : BEGIN_TOKEN END_TOKEN {$$ = strdup("BEGIN END");} 
      ;             

%%

int main (void) {
	return yyparse ( );
}

int yyerror (char *msg) {
	fprintf (stderr, "%d: %s at '%s'\n", yylineno, msg, yytext);
	return 0;
}