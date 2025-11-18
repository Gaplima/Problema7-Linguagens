%{
  #include <stdio.h>
  #include <stdlib.h>
  #include "ast.h"  // Incluimos nossa definição de AST

  int yylex(void);
  void yyerror(char *msg);

  ASTNode *root = NULL; // Raiz da árvore
%}

/* Atualizamos a union para incluir o ponteiro para o Nó da AST */
%union {
    int iValue;
    char* sValue;
    struct ASTNode* node; 
}

/* Mapeamento de tokens */
%token <iValue> NUMBER
%token <sValue> ID

%token WHILE DO IF THEN ELSE ELIF
%token BLOCK_BEGIN BLOCK_END
%token ASSIGN SEMI VALUE
%token AND OR
%token POWER
%token OP_ADD_ONE OP_SUB_ONE OP_SIZEOF
%token GREATER_THAN_OR_EQUALS LESS_THAN_OR_EQUALS EQUALS
%token GREATER_THAN LESS_THAN
%token TYPE_INT TYPE_ARRAY TYPE_CHAR TYPE_STRING TYPE_FLOAT

/* Definimos que os não-terminais retornam 'node' */
%type <node> program stmt_list stmt expr

/* Precedência */
%left GREATER_THAN LESS_THAN GREATER_THAN_OR_EQUALS LESS_THAN_OR_EQUALS EQUALS
%left '+' '-'
%left '*' '/'
%right POWER
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%

program:
    stmt_list
    {
        root = $1; // Salva a raiz
        printf("\n--- Árvore Sintática Gerada ---\n");
        print_ast(root, 0);
    }
    ;

stmt_list:
    stmt_list stmt
    {
        // Cria um nó de sequência ligando a lista anterior ao novo statement
        $$ = create_seq($1, $2);
    }
  | stmt
    {
        $$ = $1;
    }
  ;

stmt:
    IF expr THEN stmt %prec LOWER_THAN_ELSE
    {
        $$ = create_if($2, $4, NULL);
    }
  | IF expr THEN stmt ELSE stmt
    {
        $$ = create_if($2, $4, $6);
    }
  | WHILE expr DO stmt
    {
        $$ = create_while($2, $4);
    }
  | BLOCK_BEGIN stmt_list BLOCK_END
    {
        // Encapsula a lista num nó tipo BLOCK para organização visual
        ASTNode *blk = create_node(NODE_BLOCK);
        blk->left = $2;
        $$ = blk;
    }
  | ID ASSIGN expr SEMI
    {
        $$ = create_assign($1, $3);
        free($1); // O create_assign faz strdup, podemos liberar o original do lexer
    }
  | SEMI
    {
        $$ = NULL; // Representa comando vazio
    }
  ;

expr:
    expr LESS_THAN expr { $$ = create_bin_op("<", $1, $3); }
  | expr GREATER_THAN expr { $$ = create_bin_op(">", $1, $3); }
  | expr LESS_THAN_OR_EQUALS expr { $$ = create_bin_op("<=", $1, $3); }
  | expr GREATER_THAN_OR_EQUALS expr { $$ = create_bin_op(">=", $1, $3); }
  | expr EQUALS expr { $$ = create_bin_op("==", $1, $3); }
  | expr '+' expr { $$ = create_bin_op("+", $1, $3); }
  | expr '-' expr { $$ = create_bin_op("-", $1, $3); }
  | expr '*' expr { $$ = create_bin_op("*", $1, $3); }
  | expr '/' expr { $$ = create_bin_op("/", $1, $3); }
  | expr POWER expr { $$ = create_bin_op("^", $1, $3); }
  | '(' expr ')' 
    { 
        $$ = $2; // Parênteses apenas guiam a precedência, não precisam de nó na AST
    }
  | NUMBER
    {
        $$ = create_const($1);
    }
  | ID
    {
        $$ = create_var($1);
        free($1);
    }
  ;

%%

void yyerror(char *msg) {
    fprintf(stderr, "Erro de sintaxe: %s\n", msg);
}

int main(void) {
    printf("Digite código para análise:\n");
    yyparse();
    return 0;
}