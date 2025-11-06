%{
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>

  void yyerror(char *msg);
  int yylex(void);

  int indent = 0;

  void print_indent() {
      for (int i = 0; i < indent; i++) printf("  ");
  }
%}

/* Definição dos tipos usados em yylval */
%union {
    int iValue;
    char* sValue;
}

/* Tokens com e sem valores */
%token <iValue> NUMBER
%token <sValue> ID

%token WHILE DO IF THEN ELSE ELIF
%token BLOCK_BEGIN BLOCK_END
%token ASSIGN SEMI VALUE
%token AND OR
%token OP_ADD_ONE OP_SUB_ONE OP_SIZEOF
%token GREATER_THAN_OR_EQUALS LESS_THAN_OR_EQUALS EQUALS
%token TYPE_INT TYPE_ARRAY TYPE_CHAR TYPE_STRING TYPE_FLOAT


/* ==== Precedência e associatividade ==== */
%left '+' '-'
%left '*' '/'
%right POWER

// Menor procedência
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

/* Declaração inicial da gramática */
%%
program:
    stmt_list
    {
        printf("\n Programa analisado com sucesso!\n");
    }
    ;

stmt_list:
    stmt_list stmt
  | stmt
  ;

stmt:
    IF expr THEN stmt %prec LOWER_THAN_ELSE
    {
        print_indent();
        printf("→ Estrutura IF-THEN reconhecida.\n");
    }
  | IF expr THEN stmt ELSE stmt
    {
        print_indent();
        printf("→ Estrutura IF-THEN-ELSE reconhecida.\n");
    }
  | WHILE expr DO stmt
    {
        print_indent();
        printf("→ Estrutura WHILE reconhecida.\n");
    }
  | BLOCK_BEGIN stmt_list BLOCK_END
    {
        print_indent();
        printf("→ Bloco BEGIN/END reconhecido.\n");
    }
  | ID ASSIGN expr SEMI
    {
        print_indent();
        printf("→ Atribuição reconhecida: %s := (expressão)\n", $1);
        free($1);
    }
  | SEMI
    {
        print_indent();
        printf("→ Linha vazia reconhecida.\n");
    }
  ;


expr:
    expr '+' expr
    {
        print_indent(); printf("   Operação: soma.\n");
    }
  | expr '-' expr
    {
        print_indent(); printf("   Operação: subtração.\n");
    }
  | expr '*' expr
    {
        print_indent(); printf("   Operação: multiplicação.\n");
    }
  | expr '/' expr
    {
        print_indent(); printf("   Operação: divisão.\n");
    }
  | expr POWER expr
    {
      print_indent(); printf("   Operação: potenciação.\n");
    }
  | '(' expr ')'
    {
        print_indent(); printf("   Expressão entre parênteses.\n");
    }
  | NUMBER
    {
        print_indent(); printf("   Número: %d\n", $1);
    }
  | ID
    {
        print_indent(); printf("   Identificador: %s\n", $1);
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
