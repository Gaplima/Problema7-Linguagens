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
    char* sValue;       
    struct ASTNode* node; 
    int typeValue;      
}

/* Tokens */
%token <iValue> NUMBER
%token <sValue> ID

%token WHILE DO IF THEN ELSE ELIF FOR TO
%token BLOCK_BEGIN BLOCK_END
%token ASSIGN SEMI RETURN PRINT
%token AND OR POWER
%token OP_ADD_ONE OP_SUB_ONE OP_SIZEOF
%token GREATER_THAN_OR_EQUALS LESS_THAN_OR_EQUALS EQUALS
%token GREATER_THAN LESS_THAN

/* Tipos */
%token TYPE_INT TYPE_ARRAY TYPE_CHAR TYPE_STRING TYPE_FLOAT

/* Tipos dos Nós */
%type <node> program stmt_list stmt expr
%type <node> globals global_item func_def 
%type <node> declarations declaration
%type <node> params param_list param
%type <node> args arg_list
%type <typeValue> type 

/* Precedência */
%left GREATER_THAN LESS_THAN GREATER_THAN_OR_EQUALS LESS_THAN_OR_EQUALS EQUALS
%left '+' '-'
%left '*' '/'
%right POWER
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%

/* ========================================================================== */
/* ESTRUTURA GERAL                                                            */
/* ========================================================================== */

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
  ;

/* ========================================================================== */
/* DECLARAÇÕES                                                                */
/* ========================================================================== */

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
  | /* vazio */ { $$ = NULL; }
  ;

declaration:
    type ID SEMI
    {
        Symbol *s = lookup_symbol($2);
        if (s != NULL && s->scope == current_scope) {
             printf("ERRO: %s duplicado\n", $2); exit(1); 
        }
        install_symbol($2, $1, KIND_SCALAR, 0, 0);
        $$ = create_decl($2, $1, KIND_SCALAR, 0, 0);
        free($2); 
    }
  | type ID '=' '[' NUMBER ']' SEMI
    {
        install_symbol($2, $1, KIND_ARRAY, $5, 0);
        $$ = create_decl($2, $1, KIND_ARRAY, $5, 0);
        free($2);
    }
  | type ID '=' '[' NUMBER ']' '[' NUMBER ']' SEMI
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

/* ========================================================================== */
/* FUNÇÕES                                                                    */
/* ========================================================================== */

func_def:
    type ID '(' 
    { 
        install_symbol($2, $1, KIND_FUNCTION, 0, 0);
        enter_scope(); 
    }
    params ')' BLOCK_BEGIN declarations stmt_list BLOCK_END
    {
        ASTNode *body = $9;
        if ($8 != NULL) {
            body = create_seq($8, $9);
        }

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

/* ========================================================================== */
/* COMANDOS E EXPRESSÕES                                                      */
/* ========================================================================== */

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
        if (sym->kind != KIND_SCALAR) { printf("ERRO: '%s' nao escalar.\n", $1); exit(1); }
        if (sym->type != $3->dataType) { printf("AVISO: Tipos diferentes em '%s'.\n", $1); }
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
  | RETURN expr SEMI { $$ = create_return($2); }
  | SEMI { $$ = NULL; }
  ;

expr:
    expr LESS_THAN expr    { $$ = create_bin_op("<", $1, $3); $$->dataType = TYPE_INT; }
  | expr GREATER_THAN expr { $$ = create_bin_op(">", $1, $3); $$->dataType = TYPE_INT; }
  | expr LESS_THAN_OR_EQUALS expr { $$ = create_bin_op("<=", $1, $3); $$->dataType = TYPE_INT; }
  | expr GREATER_THAN_OR_EQUALS expr { $$ = create_bin_op(">=", $1, $3); $$->dataType = TYPE_INT; }
  | expr EQUALS expr { $$ = create_bin_op("==", $1, $3); $$->dataType = TYPE_INT; }
  | expr '+' expr 
    { 
        if ($1->dataType != $3->dataType) printf("ERRO: Tipos incompativeis soma\n");
        $$ = create_bin_op("+", $1, $3); $$->dataType = $1->dataType;
    }
  | expr '-' expr { $$ = create_bin_op("-", $1, $3); $$->dataType = TYPE_INT; }
  | expr '*' expr { $$ = create_bin_op("*", $1, $3); $$->dataType = TYPE_INT; }
  | NUMBER { $$ = create_const($1); $$->dataType = TYPE_INT; }
  | ID 
    {
        Symbol *sym = lookup_symbol($1);
        if (!sym) { printf("ERRO: '%s' sumiu?\n", $1); exit(1); }
        if (sym->kind != KIND_SCALAR) { printf("ERRO: '%s' e array, use []\n", $1); exit(1); }
        $$ = create_var($1);
        $$->dataType = sym->type;
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