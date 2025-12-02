%{
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include "symbol_table.h"
  #include "ast.h"

  /* Variável externa contada pelo Flex */
  extern int yylineno;

  void generate_c_code(ASTNode *root, char *filename);
  int yylex(void);
  void yyerror(char *msg);

  ASTNode *root = NULL;

  #ifndef KIND_SCALAR
    #define KIND_ARRAY  1
    #define KIND_SCALAR 0
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
%token GOTO COLON

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
  | unit_feature { $$ = $1; } 
  ;

unit_def:
    UNIT ID BLOCK_BEGIN declarations BLOCK_END
    {
        $$ = create_unit_def($2, $4);
        free($2);
    }
  ;

/* Regra Unificada para UNITs no Escopo Global/Local */
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
  | declarations unit_feature 
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
    /* Caso 1: Declaração Simples (int x;) */
    type ID SEMI
    {
        Symbol *s = lookup_symbol($2);
        /* (B) Verifica colisão no mesmo escopo */
        if (s != NULL && s->scope == current_scope) {
             printf("ERRO (Linha %d): Variavel '%s' ja declarada neste escopo.\n", yylineno, $2);
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
  | TYPE_ARRAY  { $$ = TYPE_ARRAY; }
  ;

func_def:
    type ID '(' 
    { 
        install_symbol($2, $1, KIND_FUNCTION, 0, 0);
        enter_scope(); /* Escopo 1: Parâmetros */
    }
    params ')' block_start declarations stmt_list BLOCK_END
    {
        /* block_start criou o Escopo 2 (Corpo) */
        
        ASTNode *body = $9; /* stmt_list ($9) */
        
        /* Concatena declarations ($8) com stmt_list ($9) */
        if ($8 != NULL) body = create_seq($8, $9);
        
        $$ = create_func_def($2, $1, $5, body);
        
        exit_scope(); /* Fecha Escopo 2 (Corpo) */
        exit_scope(); /* Fecha Escopo 1 (Parâmetros) */
        
        free($2);
    }
	/* --- NOVO: Opção 2: Retorno do tipo UNIT (Adicione isto) --- */
  | UNIT ID ID '(' 
    { 
        /* $2 = Nome da Unit (ex: rational_r), $3 = Nome da Função */
        install_symbol($3, 1000, KIND_FUNCTION, 0, 0);
        enter_scope(); 
    }
    params ')' block_start declarations stmt_list BLOCK_END
    {
        ASTNode *body = $10;
        /* Concatena declarações com comandos */
        if ($9 != NULL) body = create_seq($9, $10);
        
        /* Cria a função com tipo 1000 */
        $$ = create_func_def($3, 1000, $6, body);
        $$->unitName = strdup($2); /* Guarda o nome da struct retornada */
        
        exit_scope(); exit_scope();
        free($2); free($3);
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
        /* Se for TYPE_ARRAY, marcamos como KIND_ARRAY para permitir acesso r[0] */
        int kind = ($1 == TYPE_ARRAY) ? KIND_ARRAY : KIND_SCALAR;

        install_symbol($2, $1, kind, 0, 0);

        $$ = create_var($2);
        $$->dataType = $1; /* IMPORTANTE: Salva o tipo para o Codegen usar */
        free($2);
    }
  | UNIT ID ID
    {
        /* Ex: unit rational_r r1 */
        /* $2 = "rational_r", $3 = "r1" */
        install_symbol($3, 1000, KIND_SCALAR, 0, 0);
        $$ = create_var($3);
        $$->dataType = 1000;       /* Marca como tipo Unit/Struct */
        $$->unitName = strdup($2); /* Salva o nome do tipo (rational_r) */
        free($2);
        free($3);
    }
    ;

/* Regra Auxiliar para resolver conflito Shift/Reduce e abrir escopo */
block_start:
    BLOCK_BEGIN { enter_scope(); }
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
        if(!s) { 
            printf("ERRO (Linha %d): Variavel '%s' nao existe.\n", yylineno, $2); 
            exit(1); 
        }
        if(s->kind != KIND_SCALAR) { 
            printf("ERRO (Linha %d): '%s' nao e variavel escalar (iterador).\n", yylineno, $2); 
            exit(1); 
        }
        $$ = create_for($2, $4, $6, $8); free($2); 
    }
  | block_start stmt_list BLOCK_END 
    { 
        /* block_start abriu escopo, aqui fechamos */
        exit_scope();

        ASTNode *blk = create_node(NODE_BLOCK);
        blk->left = $2; 
        $$ = blk; 
    }
  /* Chamada de procedimento (Comando) - Void ou ignorando retorno */
  | ID '(' args ')' SEMI
    {
        Symbol *sym = lookup_symbol($1);
        if (!sym || sym->kind != KIND_FUNCTION) { 
            printf("ERRO (Linha %d): '%s' nao e uma funcao.\n", yylineno, $1); 
            exit(1); 
        }
        
        ASTNode *node = create_node(NODE_PROC_CALL); 
        node->strValue = $1;
        node->left = $3; /* args */
        $$ = node;
    }
  | ID ASSIGN expr SEMI
    {
        Symbol *sym = lookup_symbol($1);
        if (!sym) { 
            printf("ERRO (Linha %d): Variavel '%s' nao declarada.\n", yylineno, $1); 
            exit(1); 
        }
        if (sym->kind != KIND_SCALAR && sym->kind != KIND_UNIT) { 
             printf("ERRO (Linha %d): '%s' nao e variavel escalar ou unit.\n", yylineno, $1); 
             exit(1); 
        }
        $$ = create_assign($1, $3);
        free($1);
    }
  | ID '[' expr ']' ASSIGN expr SEMI
    {
        Symbol *sym = lookup_symbol($1);
        if (!sym || sym->kind != KIND_ARRAY) { 
            printf("ERRO (Linha %d): '%s' nao e um array.\n", yylineno, $1); 
            exit(1); 
        }
        $$ = create_assign_idx($1, $3, NULL, $6);
        free($1);
    }
  | ID '[' expr ']' '[' expr ']' ASSIGN expr SEMI
    {
        Symbol *sym = lookup_symbol($1);
        if (!sym || sym->kind != KIND_MATRIX) { 
            printf("ERRO (Linha %d): '%s' nao e uma matriz.\n", yylineno, $1); 
            exit(1); 
        }
        $$ = create_assign_idx($1, $3, $6, $9);
        free($1);
    }
  | PRINT '(' args ')' SEMI { $$ = create_print($3); }
  | READ '(' ID ')' SEMI 
    {
        Symbol *sym = lookup_symbol($3);
        if (!sym) { 
            printf("ERRO (Linha %d): Var '%s' nao declarada.\n", yylineno, $3); 
            exit(1); 
        }
        if (sym->kind != KIND_SCALAR) { 
            printf("ERRO (Linha %d): '%s' deve ser variavel simples para leitura direta.\n", yylineno, $3); 
            exit(1); 
        }
        $$ = create_read($3, sym->type);
        free($3);
    }
  | READ '(' ID '[' expr ']' ')' SEMI 
    {
        Symbol *sym = lookup_symbol($3);
        if (!sym) { 
            printf("ERRO (Linha %d): Array '%s' nao declarado.\n", yylineno, $3); 
            exit(1); 
        }
        if (sym->kind != KIND_ARRAY) { 
            printf("ERRO (Linha %d): '%s' nao e um array.\n", yylineno, $3); 
            exit(1); 
        }
        $$ = create_read_array($3, $5, sym->type);
        free($3);
    }
  | READ '(' ID '[' expr ']' '[' expr ']' ')' SEMI 
    {
        Symbol *sym = lookup_symbol($3);
        if (!sym) { 
            printf("ERRO (Linha %d): Matriz '%s' nao declarada.\n", yylineno, $3); 
            exit(1); 
        }
        if (sym->kind != KIND_MATRIX) { 
            printf("ERRO (Linha %d): '%s' nao e uma matriz.\n", yylineno, $3); 
            exit(1); 
        }
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
  | GOTO ID SEMI 
    { 
        $$ = create_goto($2); 
        free($2); 
    }

  /* 2. Definição de Rótulo: label: */
  | ID COLON 
    { 
        $$ = create_label($1); 
        free($1); 
    }
  | RETURN expr SEMI { $$ = create_return($2); }
  | SEMI { $$ = NULL; }
  ;

expr:
    /* --- OPERADORES RELACIONAIS (Retornam INT 0 ou 1) --- */
    expr LESS_THAN expr            
    { 
        if ($1->dataType == TYPE_STRING || $3->dataType == TYPE_STRING) {
             printf("ERRO (Linha %d): Nao pode comparar Strings com <.\n", yylineno); exit(1);
        }
        $$ = create_bin_op("<", $1, $3); $$->dataType = TYPE_INT; 
    }
  | expr GREATER_THAN expr         
    { 
        if ($1->dataType == TYPE_STRING || $3->dataType == TYPE_STRING) {
             printf("ERRO (Linha %d): Nao pode comparar Strings com >.\n", yylineno); exit(1);
        }
        $$ = create_bin_op(">", $1, $3); $$->dataType = TYPE_INT; 
    }
  | expr LESS_THAN_OR_EQUALS expr  
    { 
        if ($1->dataType == TYPE_STRING || $3->dataType == TYPE_STRING) {
             printf("ERRO (Linha %d): Nao pode comparar Strings com <=.\n", yylineno); exit(1);
        }
        $$ = create_bin_op("<=", $1, $3); $$->dataType = TYPE_INT; 
    }
  | expr GREATER_THAN_OR_EQUALS expr 
    { 
        if ($1->dataType == TYPE_STRING || $3->dataType == TYPE_STRING) {
             printf("ERRO (Linha %d): Nao pode comparar Strings com >=.\n", yylineno); exit(1);
        }
        $$ = create_bin_op(">=", $1, $3); $$->dataType = TYPE_INT; 
    }
  | expr EQUALS expr               
    { 
        /* Para igualdade, permitimos comparar tipos iguais. Se forem diferentes numéricos, o C resolve */
        if ($1->dataType != $3->dataType && !($1->dataType != TYPE_STRING && $3->dataType != TYPE_STRING)) {
             printf("ERRO (Linha %d): Comparacao de igualdade invalida (Tipos incompativeis).\n", yylineno); exit(1);
        }
        $$ = create_bin_op("==", $1, $3); $$->dataType = TYPE_INT; 
    }

    /* --- OPERADORES LÓGICOS --- */
  | expr AND expr                  
    { $$ = create_bin_op("&&", $1, $3); $$->dataType = TYPE_INT; }
  | expr OR expr                   
    { $$ = create_bin_op("||", $1, $3); $$->dataType = TYPE_INT; }

    /* --- POTÊNCIA (Sempre promove para Float se necessário) --- */
  | expr POWER expr  
    { 
      if ($1->dataType == TYPE_STRING || $3->dataType == TYPE_STRING) {
           printf("ERRO (Linha %d): Nao pode elevar Strings.\n", yylineno); exit(1);
      }
      
      ASTNode *L = $1;
      ASTNode *R = $3;
      
      /* Se algum for float, casta o outro para float */
      if (L->dataType == TYPE_FLOAT && R->dataType == TYPE_INT) {
           R = create_cast(R, TYPE_FLOAT);
           $$ = create_bin_op("^", L, R);
           $$->dataType = TYPE_FLOAT;
      }
      else if (L->dataType == TYPE_INT && R->dataType == TYPE_FLOAT) {
           L = create_cast(L, TYPE_FLOAT);
           $$ = create_bin_op("^", L, R);
           $$->dataType = TYPE_FLOAT;
      }
      else if (L->dataType == TYPE_FLOAT && R->dataType == TYPE_FLOAT) {
           $$ = create_bin_op("^", L, R);
           $$->dataType = TYPE_FLOAT;
      }
      else {
           /* Int ^ Int = Int (ou Float dependendo da sua regra, mantendo Int aqui) */
           $$ = create_bin_op("^", L, R);
           $$->dataType = TYPE_INT; 
      }
    }
  
  | '(' expr ')' { $$ = $2; }

    /* --- OPERADORES ARITMÉTICOS (SOMA) --- */
  | expr '+' expr 
    { 
        /* 1. Verifica Erro de String */
        if ($1->dataType == TYPE_STRING || $3->dataType == TYPE_STRING) {
            printf("ERRO (Linha %d): Nao pode somar um número a uma string", yylineno); exit(1);
        }
        
        /* 2. Coerção: INT + FLOAT -> FLOAT */
        if ($1->dataType == TYPE_INT && $3->dataType == TYPE_FLOAT) {
             $$ = create_bin_op("+", create_cast($1, TYPE_FLOAT), $3);
             $$->dataType = TYPE_FLOAT;
        }
        /* 3. Coerção: FLOAT + INT -> FLOAT */
        else if ($1->dataType == TYPE_FLOAT && $3->dataType == TYPE_INT) {
             $$ = create_bin_op("+", $1, create_cast($3, TYPE_FLOAT));
             $$->dataType = TYPE_FLOAT;
        }
        /* 4. Tipos Iguais */
        else {
             $$ = create_bin_op("+", $1, $3); 
             $$->dataType = $1->dataType;
        }
    }
    
    /* --- SUBTRAÇÃO --- */
  | expr '-' expr 
    { 
        if ($1->dataType == TYPE_STRING || $3->dataType == TYPE_STRING) {
            printf("ERRO (Linha %d): Nao pode subtrair um número de uma string\n", yylineno); exit(1);
        }
        
        if ($1->dataType == TYPE_INT && $3->dataType == TYPE_FLOAT) {
             $$ = create_bin_op("-", create_cast($1, TYPE_FLOAT), $3);
             $$->dataType = TYPE_FLOAT;
        }
        else if ($1->dataType == TYPE_FLOAT && $3->dataType == TYPE_INT) {
             $$ = create_bin_op("-", $1, create_cast($3, TYPE_FLOAT));
             $$->dataType = TYPE_FLOAT;
        }
        else {
             $$ = create_bin_op("-", $1, $3); 
             $$->dataType = $1->dataType;
        }
    }
    
    /* --- MULTIPLICAÇÃO --- */
  | expr '*' expr 
    { 
        if ($1->dataType == TYPE_STRING || $3->dataType == TYPE_STRING) {
            printf("ERRO (Linha %d): Nao pode multiplicar um número a uma string.\n", yylineno); exit(1);
        }
        
        if ($1->dataType == TYPE_INT && $3->dataType == TYPE_FLOAT) {
             $$ = create_bin_op("*", create_cast($1, TYPE_FLOAT), $3);
             $$->dataType = TYPE_FLOAT;
        }
        else if ($1->dataType == TYPE_FLOAT && $3->dataType == TYPE_INT) {
             $$ = create_bin_op("*", $1, create_cast($3, TYPE_FLOAT));
             $$->dataType = TYPE_FLOAT;
        }
        else {
             $$ = create_bin_op("*", $1, $3); 
             $$->dataType = $1->dataType;
        }
    }

    /* --- DIVISÃO --- */
  | expr '/' expr 
    { 
        if ($1->dataType == TYPE_STRING || $3->dataType == TYPE_STRING) {
            printf("ERRO (Linha %d): Nao pode dividir Strings.\n", yylineno); exit(1);
        }
        
        /* Divisão sempre tende a Float se um deles for Float. 
           Se forem dois INTs, permanece divisão inteira do C (ex: 5/2 = 2). */
           
        if ($1->dataType == TYPE_INT && $3->dataType == TYPE_FLOAT) {
             $$ = create_bin_op("/", create_cast($1, TYPE_FLOAT), $3);
             $$->dataType = TYPE_FLOAT;
        }
        else if ($1->dataType == TYPE_FLOAT && $3->dataType == TYPE_INT) {
             $$ = create_bin_op("/", $1, create_cast($3, TYPE_FLOAT));
             $$->dataType = TYPE_FLOAT;
        }
        else {
             $$ = create_bin_op("/", $1, $3); 
             $$->dataType = $1->dataType;
        }
    }

    /* --- TERMINAIS BÁSICOS --- */
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
        if (!sym) { 
            printf("ERRO (Linha %d): Variavel '%s' nao encontrada.\n", yylineno, $1);
            exit(1); 
        }
        if (sym->kind == KIND_MATRIX) { 
            printf("ERRO (Linha %d): '%s' e matriz, use [][] para acessar.\n", yylineno, $1);
            exit(1); 
        }

        $$ = create_var($1);
        $$->dataType = sym->type;
        if (sym->kind == KIND_UNIT) $$->kind = KIND_UNIT;
        
        /* Adicionamos isso para o CodeGen saber que é um array sendo passado */
        if (sym->kind == KIND_ARRAY) $$->kind = KIND_ARRAY; 

        free($1);
    }
  /* Chamada de funcao dentro de expressao (x = f()) - Retorna Valor */
  | ID '(' args ')'
    {
        Symbol *sym = lookup_symbol($1);
        if (!sym || sym->kind != KIND_FUNCTION) { 
            printf("ERRO (Linha %d): '%s' nao e funcao.\n", yylineno, $1); 
            exit(1); 
        }
        $$ = create_func_call($1, $3);
        $$->dataType = sym->type;
        free($1);
    }
  | ID '[' expr ']'
    {
        Symbol *sym = lookup_symbol($1);
        if (!sym || sym->kind != KIND_ARRAY) { 
            printf("ERRO (Linha %d): '%s' nao e um array.\n", yylineno, $1); 
            exit(1); 
        }
        $$ = create_array_access($1, $3, NULL);
        $$->dataType = sym->type; 
        free($1);
    }
  | ID '[' expr ']' '[' expr ']'
    {
        Symbol *sym = lookup_symbol($1);
        if (!sym || sym->kind != KIND_MATRIX) { 
            printf("ERRO (Linha %d): '%s' nao e uma matriz.\n", yylineno, $1); 
            exit(1); 
        }
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
    fprintf(stderr, "Erro de sintaxe na linha %d: %s\n", yylineno, msg);
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