%{
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include "symbol_table.h"
  #include "ast.h"

  int yylex(void);
  void yyerror(char *msg);

  ASTNode *root = NULL;

  /* Constantes para facilitar a leitura (caso não estejam no header) */
  #ifndef KIND_SCALAR
    #define KIND_SCALAR 0
    #define KIND_ARRAY  1
    #define KIND_MATRIX 2
  #endif
%}

%union {
    int iValue;         /* Valor inteiro para NUMBER */
    char* sValue;       /* Texto para ID */
    struct ASTNode* node; /* Nó da árvore */
    int typeValue;      /* Valor do enum de tipo (TYPE_INT, etc) */
}

/* Tokens vindos do Lexer */
%token <iValue> NUMBER
%token <sValue> ID

%token WHILE DO IF THEN ELSE ELIF
%token FOR TO
%token BLOCK_BEGIN BLOCK_END
%token ASSIGN SEMI
%token AND OR POWER
%token OP_ADD_ONE OP_SUB_ONE OP_SIZEOF
%token GREATER_THAN_OR_EQUALS LESS_THAN_OR_EQUALS EQUALS
%token GREATER_THAN LESS_THAN

/* Tokens de Tipo */
%token TYPE_INT TYPE_ARRAY TYPE_CHAR TYPE_STRING TYPE_FLOAT

/* Tipos retornados pelas regras não-terminais */
%type <node> stmt_list stmt expr
%type <node> program
%type <typeValue> type 

/* Precedência de Operadores */
%left GREATER_THAN LESS_THAN GREATER_THAN_OR_EQUALS LESS_THAN_OR_EQUALS EQUALS
%left '+' '-'
%left '*' '/'
%right POWER
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%

/* ========================================================================== */
/* PROGRAMA                                  */
/* ========================================================================== */

program:
    declarations stmt_list
    {
        root = $2; 
        printf("\n--- Tabela de Simbolos Final ---\n");
        print_symbol_table();
        printf("\n--- Arvore Sintatica Gerada ---\n");
        print_ast(root, 0);
    }
    ;

/* ========================================================================== */
/* DECLARAÇÕES                                 */
/* ========================================================================== */

declarations:
    declarations declaration
  | /* vazio - permite começar direto ou terminar declarações */
  ;

declaration:
    /* Declaração Escalar: int x; */
    type ID SEMI
    {
        if (lookup_symbol($2) != NULL) {
            printf("ERRO SEMANTICO: Variavel '%s' ja declarada!\n", $2);
            exit(1); 
        }
        install_symbol($2, $1, KIND_SCALAR, 0, 0);
        free($2); 
    }
    /* Declaração Array (Python-style): int lista = [10]; */
  | type ID '=' '[' NUMBER ']' SEMI
    {
        if (lookup_symbol($2) != NULL) {
            printf("ERRO SEMANTICO: Array '%s' ja declarado!\n", $2);
            exit(1); 
        }
        install_symbol($2, $1, KIND_ARRAY, $5, 0);
        free($2);
    }
    /* Declaração Matriz (Python-style): int matriz = [5][5]; */
  | type ID '=' '[' NUMBER ']' '[' NUMBER ']' SEMI
    {
        if (lookup_symbol($2) != NULL) {
            printf("ERRO SEMANTICO: Matriz '%s' ja declarada!\n", $2);
            exit(1); 
        }
        install_symbol($2, $1, KIND_MATRIX, $5, $8);
        free($2);
    }
  ;

type:
    TYPE_INT    { $$ = TYPE_INT; }
  | TYPE_FLOAT  { $$ = TYPE_FLOAT; }
  | TYPE_STRING { $$ = TYPE_STRING; }
  | TYPE_CHAR   { $$ = TYPE_CHAR; }
  ;

/* ========================================================================== */
/* COMANDOS                                  */
/* ========================================================================== */

stmt_list:
    stmt_list stmt { $$ = create_seq($1, $2); }
  | stmt { $$ = $1; }
  ;

stmt:
    /* Controle de Fluxo */
    IF expr THEN stmt %prec LOWER_THAN_ELSE { $$ = create_if($2, $4, NULL); }
  | IF expr THEN stmt ELSE stmt             { $$ = create_if($2, $4, $6); }
  | WHILE expr DO stmt                      { $$ = create_while($2, $4); }
  | BLOCK_BEGIN stmt_list BLOCK_END 
    { 
        ASTNode *blk = create_node(NODE_BLOCK);
        blk->left = $2;
        $$ = blk; 
    }

    /* Atribuição Simples: x := 10; */
  | ID ASSIGN expr SEMI
    {
        Symbol *sym = lookup_symbol($1);
        if (sym == NULL) {
            printf("ERRO SEMANTICO: Variavel '%s' nao declarada!\n", $1);
            exit(1);
        }
        if (sym->kind != KIND_SCALAR) {
            printf("ERRO SEMANTICO: '%s' e um array/matriz, use indices!\n", $1);
            exit(1);
        }
        // Checagem básica de tipo
        if (sym->type != $3->dataType) {
             printf("AVISO: Atribuicao de tipos diferentes em '%s'.\n", $1);
        }
        $$ = create_assign($1, $3);
        free($1);
    }
    /* Loop FOR: for i := 0 to 10 do stmt */
  | FOR ID ASSIGN expr TO expr DO stmt
    {
        Symbol *sym = lookup_symbol($2);
        
        // 1. Verifica se a variável existe
        if (sym == NULL) {
            printf("ERRO SEMANTICO: Variavel do FOR '%s' nao declarada!\n", $2);
            exit(1);
        }
        
        // 2. Verifica se é uma variável escalar (não array)
        if (sym->kind != KIND_SCALAR) {
            printf("ERRO SEMANTICO: Variavel do FOR '%s' deve ser escalar!\n", $2);
            exit(1);
        }

        // 3. Verifica se é INT (opcional, mas recomendado)
        if (sym->type != TYPE_INT) {
             printf("ERRO SEMANTICO: Variavel do FOR deve ser inteira!\n");
             exit(1);
        }

        $$ = create_for($2, $4, $6, $8);
        free($2);
    }

    /* Atribuição em Array: vetor[i] := 10; */
  | ID '[' expr ']' ASSIGN expr SEMI
    {
        Symbol *sym = lookup_symbol($1);
        if (sym == NULL || sym->kind != KIND_ARRAY) {
             printf("ERRO SEMANTICO: '%s' nao e um array valido!\n", $1);
             exit(1);
        }
        $$ = create_assign_idx($1, $3, NULL, $6);
        free($1);
    }

    /* Atribuição em Matriz: matriz[i][j] := 10; */
  | ID '[' expr ']' '[' expr ']' ASSIGN expr SEMI
    {
        Symbol *sym = lookup_symbol($1);
        if (sym == NULL || sym->kind != KIND_MATRIX) {
             printf("ERRO SEMANTICO: '%s' nao e uma matriz valida!\n", $1);
             exit(1);
        }
        $$ = create_assign_idx($1, $3, $6, $9);
        free($1);
    }

  | SEMI { $$ = NULL; }
  ;

/* ========================================================================== */
/* EXPRESSÕES                                 */
/* ========================================================================== */

expr:
    /* Operações Binárias */
    expr LESS_THAN expr 
    { 
        $$ = create_bin_op("<", $1, $3); 
        $$->dataType = TYPE_INT; 
    }
  | expr GREATER_THAN expr 
    { 
        $$ = create_bin_op(">", $1, $3); 
        $$->dataType = TYPE_INT; 
    }
  | expr '+' expr 
    { 
        if ($1->dataType != $3->dataType) {
            printf("ERRO SEMANTICO: Tipos irreconciliaveis na soma!\n");
        }
        $$ = create_bin_op("+", $1, $3); 
        $$->dataType = $1->dataType;
    }
  | expr '-' expr { $$ = create_bin_op("-", $1, $3); $$->dataType = TYPE_INT; }
  | expr '*' expr { $$ = create_bin_op("*", $1, $3); $$->dataType = TYPE_INT; }
  
    /* Terminais */
  | NUMBER
    {
        $$ = create_const($1);
        $$->dataType = TYPE_INT;
    }
    
    /* Leitura de Variável Simples */
  | ID 
    {
        Symbol *sym = lookup_symbol($1);
        if (sym == NULL) {
            printf("ERRO: Variavel '%s' nao declarada!\n", $1);
            exit(1);
        }
        if (sym->kind != KIND_SCALAR) {
            printf("ERRO: '%s' e um array, especifique o indice!\n", $1);
            exit(1);
        }
        $$ = create_var($1);
        $$->dataType = sym->type;
        free($1);
    }

    /* Leitura de Array: vetor[i] */
  | ID '[' expr ']'
    {
        Symbol *sym = lookup_symbol($1);
        if (sym == NULL || sym->kind != KIND_ARRAY) {
             printf("ERRO: '%s' nao e um array para leitura!\n", $1);
             exit(1);
        }
        $$ = create_array_access($1, $3, NULL);
        $$->dataType = sym->type; 
        free($1);
    }

    /* Leitura de Matriz: matriz[i][j] */
  | ID '[' expr ']' '[' expr ']'
    {
        Symbol *sym = lookup_symbol($1);
        if (sym == NULL || sym->kind != KIND_MATRIX) {
             printf("ERRO: '%s' nao e uma matriz para leitura!\n", $1);
             exit(1);
        }
        $$ = create_array_access($1, $3, $6);
        $$->dataType = sym->type; 
        free($1);
    }
  ;

%%

void yyerror(char *msg) {
    fprintf(stderr, "Erro de sintaxe: %s\n", msg);
}

int main(void) {
    init_symbol_table();
    printf("Digite o codigo:\n");
    yyparse();
    return 0;
}