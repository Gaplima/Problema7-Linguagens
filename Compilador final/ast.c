#include "ast.h"
#include "y.tab.h"
#include "symbol_table.h"

/* 
 * CONSTRUTOR GENÉRICO (FÁBRICA BASE)
 * 
 * Aloca memória para um novo nó na Heap e inicializa todos os ponteiros como NULL.
 * Todos os outros create_* chamam esta função primeiro.
 */
ASTNode* create_node(NodeType type) {
    ASTNode *node = (ASTNode*) malloc(sizeof(ASTNode));
    node->type = type;
    node->left = NULL;
    node->right = NULL;
    node->extra = NULL;
    node->strValue = NULL;
    return node;
}

/* --- NÓS FOLHA (Terminais com valores) --- */

ASTNode* create_const(int val) {
    ASTNode *node = create_node(NODE_CONST);
    node->intValue = val; /* Armazena inteiro */
    return node;
}

ASTNode* create_var(char *name) {
    ASTNode *node = create_node(NODE_VAR);
    /* strdup: copia a string para uma nova área de memória.
       Sem isso, perderíamos o nome da variável quando o buffer do Lexer mudasse. */
    node->strValue = strdup(name); 
    return node;
}

ASTNode* create_float_const(float val) {
    ASTNode *node = create_node(NODE_CONST);
    node->floatValue = val;
    node->dataType = 290; // (Idealmente usar TYPE_FLOAT do enum)
    return node;
}

/* --- OPERAÇÕES E ATRIBUIÇÕES --- */

ASTNode* create_assign(char *varName, ASTNode *expr) {
    ASTNode *node = create_node(NODE_ASSIGN);
    node->strValue = strdup(varName); /* Quem recebe (lado esquerdo) */
    node->left = expr;                /* O valor (lado direito) */
    return node;
}

ASTNode* create_bin_op(char *op, ASTNode *left, ASTNode *right) {
    ASTNode *node = create_node(NODE_BIN_OP);
    node->strValue = strdup(op); /* Guarda o operador: "+", "-", "*", etc. */
    node->left = left;
    node->right = right;
    return node;
}

/* --- CONTROLE DE FLUXO (IF, WHILE, FOR) --- */

ASTNode* create_if(ASTNode *cond, ASTNode *thenStmt, ASTNode *elseStmt) {
    ASTNode *node = create_node(NODE_IF);
    node->left = cond;      /* A condição booleana */
    node->right = thenStmt; /* Bloco executado se VERDADEIRO */
    node->extra = elseStmt; /* Bloco executado se FALSO (pode ser NULL) */
    return node;
}

ASTNode* create_while(ASTNode *cond, ASTNode *body) {
    ASTNode *node = create_node(NODE_WHILE);
    node->left = cond;      /* Condição de parada */
    node->right = body;     /* Corpo do loop */
    return node;
}

/* O FOR é complexo, precisa de 3 "filhos": Início, Fim, Corpo */
ASTNode* create_for(char *varName, ASTNode *start, ASTNode *end, ASTNode *body) {
    ASTNode *node = create_node(NODE_FOR);
    node->strValue = strdup(varName); /* Variável iteradora (ex: 'i') */
    node->left = start;               /* Valor inicial */
    node->right = end;                /* Valor final */
    node->extra = body;               /* Corpo do loop */
    return node;
}

ASTNode* create_goto(char *labelName) {
    ASTNode *node = create_node(NODE_GOTO);
    node->strValue = strdup(labelName);
    return node;
}

ASTNode* create_label(char *labelName) {
    ASTNode *node = create_node(NODE_LABEL);
    node->strValue = strdup(labelName);
    return node;
}

/* --- DECLARAÇÕES --- */

ASTNode* create_decl(char *name, int type, int kind, int size1, int size2) {
    ASTNode *node = create_node(NODE_DECL);
    node->strValue = strdup(name);
    node->dataType = type; /* INT, FLOAT, STRING */
    node->kind = kind;     /* SCALAR, ARRAY, MATRIX */
    node->size1 = size1;   /* Tamanho dimensão 1 */
    node->size2 = size2;   /* Tamanho dimensão 2 */
    return node;
}

/* NODE_SEQ: A função que une comandos numa lista encadeada */
ASTNode* create_seq(ASTNode *stmt1, ASTNode *stmt2) {
    ASTNode *node = create_node(NODE_SEQ);
    node->left = stmt1;  /* Comando atual */
    node->right = stmt2; /* Próximo(s) comando(s) */
    return node;
}

/* --- FUNÇÕES ESPECÍFICAS (I/O, Arrays, Structs) --- */

ASTNode* create_print(ASTNode *args) {
    ASTNode *node = create_node(NODE_PRINT);
    node->left = args; 
    return node;
}

ASTNode* create_read(char *varName, int type) {
    ASTNode *node = create_node(NODE_READ);
    node->strValue = strdup(varName); 
    node->dataType = type;            
    return node;
}

ASTNode* create_array_access(char *name, ASTNode *idx1, ASTNode *idx2) {
    ASTNode *node = create_node(NODE_ARRAY_ACCESS);
    node->strValue = strdup(name);
    node->left = idx1;  /* Índice da linha (ou vetor simples) */
    node->right = idx2; /* Índice da coluna (se for matriz) */
    return node;
}

/* Variação de leitura específica para Arrays (guarda o índice) */
ASTNode* create_read_array(char *varName, ASTNode *index, int type) {
    ASTNode *node = create_node(NODE_READ);
    node->strValue = strdup(varName);
    node->dataType = type;
    node->kind = KIND_ARRAY; 
    node->left = index;      
    return node;
}

/* Variação de leitura específica para Matrizes */
ASTNode* create_read_matrix(char *varName, ASTNode *row, ASTNode *col, int type) {
    ASTNode *node = create_node(NODE_READ);
    node->strValue = strdup(varName);
    node->dataType = type;
    node->kind = KIND_MATRIX; 
    node->left = row;         
    node->right = col;        
    return node;
}

/* Atribuição em índice: arr[x] = y */
ASTNode* create_assign_idx(char *name, ASTNode *idx1, ASTNode *idx2, ASTNode *val) {
    ASTNode *node = create_node(NODE_ASSIGN_IDX);
    node->strValue = strdup(name);
    node->left = idx1;    /* Índice 1 */
    node->right = idx2;   /* Índice 2 (pode ser NULL) */
    node->extra = val;    /* Valor a ser atribuído */
    return node;
}

/* Definição de Struct (Unit) */
ASTNode* create_unit_def(char *name, ASTNode *fields) {
    ASTNode *node = create_node(NODE_UNIT_DEF);
    node->strValue = strdup(name);
    node->left = fields; /* Lista de declarações internas */
    return node;
}

/* Acesso a campos: variavel.campo */
ASTNode* create_access(char *var, char *field) {
    ASTNode *node = create_node(NODE_ACCESS);
    node->strValue = strdup(var); // Nome da variável pai
    /* Usa o ponteiro 'extra' para guardar o nome do campo como um nó VAR */
    node->extra = create_node(NODE_VAR);
    node->extra->strValue = strdup(field); 
    return node;
}

/* --- FUNÇÕES E CHAMADAS --- */

ASTNode* create_func_def(char *name, int retType, ASTNode *params, ASTNode *body) {
    ASTNode *node = create_node(NODE_FUNC_DEF);
    node->strValue = strdup(name);
    node->dataType = retType;
    node->left = params;  /* Lista de parâmetros */
    node->right = body;   /* Bloco de código da função */
    return node;
}

ASTNode* create_func_call(char *name, ASTNode *args) {
    ASTNode *node = create_node(NODE_FUNC_CALL);
    node->strValue = strdup(name);
    node->left = args; /* Argumentos passados */
    return node;
}

/* Nó de Conversão de Tipo (Cast) */
ASTNode* create_cast(ASTNode *expr, int targetType) {
    ASTNode *node = create_node(NODE_CAST);
    node->left = expr;           /* A expressão original */
    node->dataType = targetType; /* O tipo para o qual vai converter */
    return node;
}

ASTNode* create_return(ASTNode *expr) {
    ASTNode *node = create_node(NODE_RETURN);
    node->left = expr;
    return node;
}

/* Criação de listas encadeadas para parâmetros e argumentos */
ASTNode* create_param_list(ASTNode *param, ASTNode *next) {
    ASTNode *node = create_node(NODE_PARAM_LIST);
    node->left = param;
    node->right = next;
    return node;
}

ASTNode* create_arg_list(ASTNode *arg, ASTNode *next) {
    ASTNode *node = create_node(NODE_ARG_LIST);
    node->left = arg;
    node->right = next;
    return node;
}

/* 
 * VISUALIZAÇÃO DA ÁRVORE (DEBUG)
 * Percorre a árvore recursivamente e imprime a estrutura hierárquica.
 * 'level' controla a indentação para mostrar quem é filho de quem.
 */

void print_indent(int level) {
    for (int i = 0; i < level; i++) printf("  ");
}

void print_ast(ASTNode *node, int level) {
    if (!node) return;

    print_indent(level);

    switch (node->type) {
        case NODE_PRINT:
            printf("PRINT:\n");
            print_indent(level+1); printf("Args:\n");
            print_ast(node->left, level+2);
            break;
            
        case NODE_CONST:
            if (node->dataType == TYPE_FLOAT) {
                printf("Float: %f\n", node->floatValue);
            } 
            else if (node->dataType == TYPE_STRING) {
                printf("String: %s\n", node->strValue);
            } 
            else {
                printf("Num: %d\n", node->intValue);
            }
            break;
            
        case NODE_VAR:
            printf("Var: %s\n", node->strValue);
            break;
            
        case NODE_ASSIGN:
            printf("Assign: %s :=\n", node->strValue);
            print_ast(node->left, level + 1);
            break;
            
        case NODE_BIN_OP:
            printf("Op: %s\n", node->strValue);
            print_ast(node->left, level + 1);
            print_ast(node->right, level + 1);
            break;
            
        case NODE_IF:
            printf("IF\n");
            print_indent(level + 1); printf("Cond:\n");
            print_ast(node->left, level + 2);
            print_indent(level + 1); printf("Then:\n");
            print_ast(node->right, level + 2);
            if (node->extra) {
                print_indent(level + 1); printf("Else:\n");
                print_ast(node->extra, level + 2);
            }
            break;
            
        case NODE_WHILE:
            printf("WHILE\n");
            print_ast(node->left, level + 1);
            print_indent(level + 1); printf("Do:\n");
            print_ast(node->right, level + 2);
            break;
            
        case NODE_FOR:
            printf("FOR Var: %s\n", node->strValue);
            print_indent(level + 1); printf("Start:\n");
            print_ast(node->left, level + 2);
            print_indent(level + 1); printf("To:\n");
            print_ast(node->right, level + 2);
            print_indent(level + 1); printf("Do:\n");
            print_ast(node->extra, level + 2);
            break;
            
        case NODE_BLOCK:
            printf("BLOCK\n");
            print_ast(node->left, level + 1);
            break;
            
        case NODE_SEQ:
            /* Note que SEQ não imprime "SEQ", apenas processa os filhos
               para deixar a visualização mais limpa */
            print_ast(node->left, level);
            if (node->right) print_ast(node->right, level); 
            break;
            
        case NODE_ARRAY_ACCESS:
            printf("Access Array: %s\n", node->strValue);
            print_indent(level+1); printf("Index 1:\n");
            print_ast(node->left, level+2);
            if(node->right) {
                 print_indent(level+1); printf("Index 2:\n");
                 print_ast(node->right, level+2);
            }
            break;
            
        case NODE_ASSIGN_IDX:
            printf("Assign Array: %s [...] :=\n", node->strValue);
            print_indent(level+1); printf("Index 1:\n");
            print_ast(node->left, level+2);
            if(node->right) {
                 print_indent(level+1); printf("Index 2:\n");
                 print_ast(node->right, level+2);
            }
            print_indent(level+1); printf("Value:\n");
            print_ast(node->extra, level+2);
            break;
            
        case NODE_FUNC_DEF:
            printf("FUNCTION: %s (Type: %d)\n", node->strValue, node->dataType);
            print_indent(level+1); printf("Params:\n");
            print_ast(node->left, level+2);
            print_indent(level+1); printf("Body:\n");
            print_ast(node->right, level+2);
            break;
            
        case NODE_PARAM_LIST:
            printf("Param:\n");
            print_ast(node->left, level+1);
            if(node->right) print_ast(node->right, level);
            break;
            
        case NODE_RETURN:
            printf("RETURN:\n");
            print_ast(node->left, level+1);
            break;
            
        case NODE_FUNC_CALL:
            printf("CALL: %s(...)\n", node->strValue);
            print_indent(level+1); printf("Args:\n");
            print_ast(node->left, level+2);
            break;
            
        case NODE_ARG_LIST:
            printf("Arg:\n");
            print_ast(node->left, level+1);
            if(node->right) print_ast(node->right, level);
            break;
    }
}