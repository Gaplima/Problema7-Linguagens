%{
#include <stdio.h>

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
%token <iValue> NUMBER
%token WHILE BLOCK_BEGIN BLOCK_END DO IF THEN ELSE SEMI ASSIGN TYPE

%start prog

%%

prog : stmtlist 
	 ;

stmtlist : stmt
         | stmt stmtlist
         ;

stmt : mixedlist ASSIGN ID {printf("atribuição\n");}
     | ids ':' TYPE        {printf("declaração\n");}
     ;
	
ids : ID	     
    | ID ',' ids 
    ;

atm : ID dms
    ;

dms : '[' ']'
    | '[' ']' dms
    ;

mixedlist : ID ',' mixedlist
          | atm ',' mixedlist
          | ID
          | atm
          ;

%%

int main (void) {
	return yyparse ( );
}

int yyerror (char *msg) {
	fprintf (stderr, "%d: %s at '%s'\n", yylineno, msg, yytext);
	return 0;
}