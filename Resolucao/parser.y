%{
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>

  void yyerror(char *msg);
  int yylex(void);
%}

/* ==== Definição dos tipos usados em yylval ==== */
%union {
    int iValue;
    char* sValue;
}

/* ==== Tokens com e sem valores ==== */
%token <iValue> NUMBER
%token <sValue> ID

%token WHILE DO IF THEN ELSE ELIF
%token BLOCK_BEGIN BLOCK_END
%token ASSIGN SEMI VALUE
%token AND OR
%token OP_ADD_ONE OP_SUB_ONE OP_SIZEOF
%token GREATER_THAN_OR_EQUALS LESS_THAN_OR_EQUALS EQUALS
%token TYPE_INT TYPE_ARRAY TYPE_CHAR TYPE_STRING TYPE_FLOAT

/* ==== Declaração inicial da gramática ==== */
%%
program:
    stmt_list
    ;

stmt_list:
    stmt_list stmt
  | stmt
  ;

stmt:
    WHILE expr DO stmt
  | IF expr THEN stmt
  | IF expr THEN stmt ELSE stmt
  | BLOCK_BEGIN stmt_list BLOCK_END
  | ID ASSIGN expr SEMI
  | SEMI
  ;

expr:
    expr '+' expr
  | expr '-' expr
  | expr '*' expr
  | expr '/' expr
  | '(' expr ')'
  | NUMBER
  | ID
  ;
%%

void yyerror(char *msg) {
    fprintf(stderr, "Erro de sintaxe: %s\n", msg);
}

int main(void) {
    printf("Digite código para análise:\n");
    yyparse();
    printf("Análise concluída com sucesso!\n");
    return 0;
}
