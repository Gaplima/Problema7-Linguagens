%{
#include <stdio.h>

int yylex(void);
int yyerror(char *s);
extern int yylineno;
extern char * yytext;

%}

%union {
	char * sValue;  /* string value */
	};

%token <sValue> ID
%token BACK WHILE ASSIGN 

%start prog

%%
prog : stmlist {printf("PROG\n");} 
	 ;

stmlist : stm          {printf("STM\n");}
		| stm stmlist  {printf("STM-STMLIST\n");}
	    ;	 

stm : ID ASSIGN ID       {printf("%s ASSIGN %s\n", $1, $3);
						  free($1);
						  free($3);}
    | WHILE stmlist BACK {printf("WHILE\n");}
	;
	

%%

int main (void) {
	return yyparse ( );
}

int yyerror (char *msg) {
	fprintf (stderr, "%d: %s at '%s'\n", yylineno, msg, yytext);
	return 0;
}