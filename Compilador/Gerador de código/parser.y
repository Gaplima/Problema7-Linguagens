%{
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include "symbol_table.h"
  #include "ast.h"

  void generate_c_code(ASTNode *root, char *filename);
  int yylex(void);
  void yyerror(char *msg);

  ASTNode *root = NULL;

  #ifndef KIND_SCALAR
    #define KIND_SCALAR 0
    #define KIND_ARRAY  1
    #define KIND_MATRIX 2
    #define KIND_FUNCTION 3
  #endif
%}

%union {
    int iValue;
    float fValue;
    char* sValue;       
    struct ASTNode* node; 
    int typeValue;      
}

/* Tokens */
%token <iValue> NUMBER
%token <sValue> ID
%token <sValue> STRING_LITERAL 
%token <fValue> FLOAT_LITERAL

%token UNIT DOT
%token WHILE DO IF THEN ELSE ELIF FOR TO
%token BLOCK_BEGIN BLOCK_END
%token ASSIGN SEMI RETURN PRINT READ
%token AND OR POWER
%token OP_ADD_ONE OP_SUB_ONE OP_SIZEOF
%token GREATER_THAN_OR_EQUALS LESS_THAN_OR_EQUALS EQUALS
%token GREATER_THAN LESS_THAN

/* Tipos */
%token TYPE_INT TYPE_ARRAY TYPE_CHAR TYPE_STRING TYPE_FLOAT

/* Tipos dos Nós */
%type <node> program stmt_list stmt expr
%type <node> globals global_item func_def unit_def unit_feature
%type <node> declarations declaration
%type <node> params param_list param
%type <node> args arg_list
%type <typeValue> type 

/* Precedência */
%left OR
%left AND
%left GREATER_THAN LESS_THAN GREATER_THAN_OR_EQUALS LESS_THAN_OR_EQUALS EQUALS
%left '+' '-'
%left '*' '/'
%right POWER
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%

program:
    globals stmt_list
    {
        ASTNode *mainBlock = create_node(NODE_BLOCK);
        mainBlock->left = $2; 
        
        if ($1 != NULL) {
            root = create_seq($1, mainBlock);
        } else {
            root = mainBlock;
        }
        
        printf("\n--- Tabela de Simbolos Global Final ---\n");
        print_symbol_table();
        printf("\n--- Arvore Sintatica Gerada ---\n");
        print_ast(root, 0);
    }
    ;

globals:
    globals global_item 
    { 
        if ($2 != NULL) {
            if ($1 == NULL) $$ = $2;
            else $$ = create_seq($1, $2);
        } else {
            $$ = $1;
        }
    }
  | /* vazio */ { $$ = NULL; }
  ;

global_item:
    declaration { $$ = $1; }
  | func_def    { $$ = $1; }
  | unit_def    { $$ = $1; }
  | unit_feature { $$ = $1; } /* NOVA REGRA PARA RESOLVER CONFLITO */
  ;

unit_def:
    UNIT ID BLOCK_BEGIN declarations BLOCK_END
    {
        $$ = create_unit_def($2, $4);
        free($2);
    }
  ;

/* Regra Unificada para UNITs no Escopo Global/Local */
/* Resolve a ambiguidade entre "unit A x;" e "unit A f()" */
unit_feature:
    /* 1. Declaração de Variável Unit: unit Ponto p1; */
    UNIT ID ID SEMI
    {
        install_symbol($3, 1000, KIND_UNIT, 0, 0);
        ASTNode *node = create_decl($3, 1000, KIND_UNIT, 0, 0);
        node->unitName = strdup($2);
        $$ = node;
        free($2); free($3);
    }
    /* 2. Definição de Função Unit: unit Ponto criar() ... */
  | UNIT ID ID '(' 
    { 
        install_symbol($3, 1000, KIND_FUNCTION, 0, 0);
        enter_scope(); 
    }
    params ')' BLOCK_BEGIN declarations stmt_list BLOCK_END
    {
        ASTNode *body = $10;
        if ($9 != NULL) body = create_seq($9, $10);
        
        $$ = create_func_def($3, 1000, $6, body);
        $$->unitName = strdup($2); 
        
        exit_scope();
        free($2); free($3);
    }
  ;

declarations:
    declarations declaration
    {
        if ($2 != NULL) {
            if ($1 == NULL) $$ = $2;
            else $$ = create_seq($1, $2);
        } else {
            $$ = $1;
        }
    }
  /* Adicionamos unit_feature aqui também para permitir variaveis locais unit */
  | declarations unit_feature 
    {
        /* Nota: unit_feature também casa com função, mas semanticamente */
        /* não deveríamos definir funções dentro de funções. O Parser aceitará sintaticamente. */
        if ($2 != NULL) {
            if ($1 == NULL) $$ = $2;
            else $$ = create_seq($1, $2);
        } else {
            $$ = $1;
        }
    }
  | /* vazio */ { $$ = NULL; }
  ;

declaration:
    /* Caso 1: Declaração Simples (int x;) */
    type ID SEMI
    {
        Symbol *s = lookup_symbol($2);
        if (s != NULL && s->scope == current_scope) {
             printf("ERRO: %s duplicado\n", $2);
             exit(1); 
        }
        install_symbol($2, $1, KIND_SCALAR, 0, 0);
        $$ = create_decl($2, $1, KIND_SCALAR, 0, 0);
        free($2); 
    }
  /* Caso 2: Array (int v := [10];) */
  | type ID ASSIGN '[' NUMBER ']' SEMI
    {
        install_symbol($2, $1, KIND_ARRAY, $5, 0);
        $$ = create_decl($2, $1, KIND_ARRAY, $5, 0);
        free($2);
    }
  /* Caso 3: Matriz (int m := [10][10];) */
  | type ID ASSIGN '[' NUMBER ']' '[' NUMBER ']' SEMI
    {
        install_symbol($2, $1, KIND_MATRIX, $5, $8);
        $$ = create_decl($2, $1, KIND_MATRIX, $5, $8);
        free($2);
    }
  ;

type:
    TYPE_INT    { $$ = TYPE_INT; }
  | TYPE_FLOAT  { $$ = TYPE_FLOAT; }
  | TYPE_STRING { $$ = TYPE_STRING; }
  | TYPE_CHAR   { $$ = TYPE_CHAR; }
  ;

func_def:
    /* Apenas funções de tipos básicos (int, float...) */
    type ID '(' 
    { 
        install_symbol($2, $1, KIND_FUNCTION, 0, 0);
        enter_scope(); 
    }
    params ')' BLOCK_BEGIN declarations stmt_list BLOCK_END
    {
        ASTNode *body = $9;
        if ($8 != NULL) body = create_seq($8, $9);
        $$ = create_func_def($2, $1, $5, body);
        exit_scope();
        free($2);
    }
  ;

params:
    param_list { $$ = $1; }
  | /* vazio */ { $$ = NULL; }
  ;

param_list:
    param_list ',' param { $$ = create_param_list($3, $1); }
  | param                { $$ = create_param_list($1, NULL); }
  ;

param:
    type ID
    {
        install_symbol($2, $1, KIND_SCALAR, 0, 0);
        $$ = create_var($2);
        free($2);
    }
    ;

stmt_list:
    stmt_list stmt { $$ = create_seq($1, $2); }
  | stmt { $$ = $1; }
  ;

stmt:
    IF expr THEN stmt %prec LOWER_THAN_ELSE { $$ = create_if($2, $4, NULL); }
  | IF expr THEN stmt ELSE stmt             { $$ = create_if($2, $4, $6); }
  | WHILE expr DO stmt                      { $$ = create_while($2, $4); }
  | FOR ID ASSIGN expr TO expr DO stmt      
    { 
        Symbol *s = lookup_symbol($2);
        if(!s) { printf("Erro: %s nao existe\n", $2); exit(1); }
        if(s->kind != KIND_SCALAR) { printf("Erro: %s nao escalar\n", $2); exit(1); }
        $$ = create_for($2, $4, $6, $8); free($2); 
    }
  | BLOCK_BEGIN stmt_list BLOCK_END 
    { 
        ASTNode *blk = create_node(NODE_BLOCK);
        blk->left = $2;
        $$ = blk; 
    }
  | ID ASSIGN expr SEMI
    {
        Symbol *sym = lookup_symbol($1);
        if (!sym) { printf("ERRO: '%s' nao declarado.\n", $1); exit(1); }
        if (sym->kind != KIND_SCALAR && sym->kind != KIND_UNIT) { 
             printf("ERRO: '%s' nao escalar/unit.\n", $1); exit(1); 
        }
        $$ = create_assign($1, $3);
        free($1);
    }
  | ID '[' expr ']' ASSIGN expr SEMI
    {
        Symbol *sym = lookup_symbol($1);
        if (!sym || sym->kind != KIND_ARRAY) { printf("ERRO: '%s' nao array.\n", $1); exit(1); }
        $$ = create_assign_idx($1, $3, NULL, $6);
        free($1);
    }
  | ID '[' expr ']' '[' expr ']' ASSIGN expr SEMI
    {
        Symbol *sym = lookup_symbol($1);
        if (!sym || sym->kind != KIND_MATRIX) { printf("ERRO: '%s' nao matriz.\n", $1); exit(1); }
        $$ = create_assign_idx($1, $3, $6, $9);
        free($1);
    }
  | PRINT '(' args ')' SEMI { $$ = create_print($3); }
  | READ '(' ID ')' SEMI 
    {
        Symbol *sym = lookup_symbol($3);
        if (!sym) { printf("ERRO: Var '%s' nao declarada.\n", $3); exit(1); }
        if (sym->kind != KIND_SCALAR) { printf("ERRO: '%s' deve ser variavel simples.\n", $3); exit(1); }
        $$ = create_read($3, sym->type);
        free($3);
    }
  | READ '(' ID '[' expr ']' ')' SEMI 
    {
        Symbol *sym = lookup_symbol($3);
        if (!sym) { printf("ERRO: Array '%s' nao declarado.\n", $3); exit(1); }
        if (sym->kind != KIND_ARRAY) { printf("ERRO: '%s' nao e array.\n", $3); exit(1); }
        $$ = create_read_array($3, $5, sym->type);
        free($3);
    }
  | READ '(' ID '[' expr ']' '[' expr ']' ')' SEMI 
    {
        Symbol *sym = lookup_symbol($3);
        if (!sym) { printf("ERRO: Matriz '%s' nao declarada.\n", $3); exit(1); }
        if (sym->kind != KIND_MATRIX) { printf("ERRO: '%s' nao e matriz.\n", $3); exit(1); }
        $$ = create_read_matrix($3, $5, $8, sym->type);
        free($3);
    }
  | ID DOT ID ASSIGN expr SEMI
    {
         ASTNode *acc = create_access($1, $3);
         ASTNode *assign = create_node(NODE_ASSIGN);
         assign->strValue = strdup("ASSIGN_ACCESS");
         assign->left = acc;
         assign->right = $5;
         $$ = assign;
         free($1); free($3);
    }
  | RETURN expr SEMI { $$ = create_return($2); }
  | SEMI { $$ = NULL; }
  ;

expr:
    expr LESS_THAN expr    { $$ = create_bin_op("<", $1, $3); $$->dataType = TYPE_INT; }
  | expr GREATER_THAN expr { $$ = create_bin_op(">", $1, $3); $$->dataType = TYPE_INT; }
  | expr LESS_THAN_OR_EQUALS expr { $$ = create_bin_op("<=", $1, $3); $$->dataType = TYPE_INT; }
  | expr GREATER_THAN_OR_EQUALS expr { $$ = create_bin_op(">=", $1, $3); $$->dataType = TYPE_INT; }
  | expr EQUALS expr { $$ = create_bin_op("==", $1, $3); $$->dataType = TYPE_INT; }
  | expr AND expr    { $$ = create_bin_op("&&", $1, $3); $$->dataType = TYPE_INT; }
  | expr OR expr     { $$ = create_bin_op("||", $1, $3); $$->dataType = TYPE_INT; }
  | expr POWER expr  { 
      $$ = create_bin_op("^", $1, $3);
      if ($1->dataType == TYPE_FLOAT || $3->dataType == TYPE_FLOAT)
          $$->dataType = TYPE_FLOAT;
      else
          $$->dataType = TYPE_INT; 
  }
  | '(' expr ')' { $$ = $2; }
  | expr '+' expr 
    { 
        if ($1->dataType != $3->dataType) printf("ERRO: Tipos incompativeis soma\n");
        $$ = create_bin_op("+", $1, $3); $$->dataType = $1->dataType;
    }
  | expr '-' expr { $$ = create_bin_op("-", $1, $3); $$->dataType = TYPE_INT; }
  | expr '*' expr { $$ = create_bin_op("*", $1, $3); $$->dataType = TYPE_INT; }
  | NUMBER { $$ = create_const($1); $$->dataType = TYPE_INT; }
  | STRING_LITERAL 
    { 
        $$ = create_node(NODE_CONST);
        $$->strValue = $1; 
        $$->dataType = TYPE_STRING;
    }
  | FLOAT_LITERAL 
    { 
        $$ = create_float_const($1);
        $$->dataType = TYPE_FLOAT; 
    }
  | ID DOT ID 
    { 
        $$ = create_access($1, $3);
        $$->dataType = TYPE_INT; 
        free($1); free($3);
    }
  | ID 
    {
        Symbol *sym = lookup_symbol($1);
        if (!sym) { printf("ERRO: '%s' sumiu?\n", $1); exit(1); }
        if (sym->kind != KIND_SCALAR && sym->kind != KIND_UNIT) { printf("ERRO: '%s' e array, use []\n", $1); exit(1); }
        $$ = create_var($1);
        $$->dataType = sym->type;
        if (sym->kind == KIND_UNIT) $$->kind = KIND_UNIT;
        free($1);
    }
  | ID '(' args ')'
    {
        Symbol *sym = lookup_symbol($1);
        if (!sym || sym->kind != KIND_FUNCTION) { printf("ERRO: '%s' nao e funcao.\n", $1); exit(1); }
        $$ = create_func_call($1, $3);
        $$->dataType = sym->type;
        free($1);
    }
  | ID '[' expr ']'
    {
        Symbol *sym = lookup_symbol($1);
        if (!sym || sym->kind != KIND_ARRAY) { printf("ERRO: '%s' nao array.\n", $1); exit(1); }
        $$ = create_array_access($1, $3, NULL);
        $$->dataType = sym->type; 
        free($1);
    }
  | ID '[' expr ']' '[' expr ']'
    {
        Symbol *sym = lookup_symbol($1);
        if (!sym || sym->kind != KIND_MATRIX) { printf("ERRO: '%s' nao matriz.\n", $1); exit(1); }
        $$ = create_array_access($1, $3, $6);
        $$->dataType = sym->type; 
        free($1);
    }
  ;

args:
    arg_list { $$ = $1; }
  | /* vazio */ { $$ = NULL; }
  ;

arg_list:
    arg_list ',' expr { $$ = create_arg_list($3, $1); }
  | expr              { $$ = create_arg_list($1, NULL); }
  ;

%%

void yyerror(char *msg) {
    fprintf(stderr, "Erro de sintaxe: %s\n", msg);
}

extern FILE *yyin;
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <arquivo_entrada>\n", argv[0]);
        return 1;
    }

    FILE *myfile = fopen(argv[1], "r");
    if (!myfile) {
        printf("Erro ao abrir arquivo: %s\n", argv[1]);
        return 1;
    }

    yyin = myfile;
    init_symbol_table();
    
    yyparse();
    
    if (root != NULL) {
        generate_c_code(root, argv[1]);
    }
    
    fclose(myfile);
    return 0;
}