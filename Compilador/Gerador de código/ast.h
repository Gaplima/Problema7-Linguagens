// ast.h
#ifndef AST_H
#define AST_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Tipos de nós (Estrutura da árvore)
typedef enum {
    NODE_CONST,
    NODE_VAR,
    NODE_ASSIGN,
    NODE_IF,
    NODE_WHILE,
    NODE_FOR,
    NODE_DECL,
    NODE_BLOCK,
    NODE_BIN_OP,
    NODE_SEQ,
    NODE_PRINT,
    NODE_READ,
    NODE_ARRAY_ACCESS, // Para ler: x[i]
    NODE_ASSIGN_IDX,   // Para atribuir: x[i] := 10
    NODE_FUNC_DEF,     // Definição de Função
    NODE_FUNC_CALL,    // Chamada de Função
    NODE_PROC_CALL,    
    NODE_RETURN,       // Return
    NODE_PARAM_LIST,   // Lista de Parâmetros
    NODE_ARG_LIST,      // Lista de Argumentos (na chamada)
    NODE_UNIT_DEF,     // Definição da Unit (o molde)
    NODE_ACCESS 
} NodeType;

// Estrutura do Nó da Árvore
typedef struct ASTNode {
    NodeType type;      // Tipo estrutural (se é IF, WHILE, etc)
    int dataType;       // Tipo de dado (TYPE_INT, TYPE_FLOAT, etc - vindo do Bison)
    float floatValue;
    int intValue;       
    char *strValue;     
    int kind;   // 0=Escalar, 1=Array, 2=Matriz
    int size1;
    int size2;
    char *unitName;

    struct ASTNode *left;
    struct ASTNode *right;
    struct ASTNode *extra;
} ASTNode;

ASTNode* create_node(NodeType type);
ASTNode* create_const(int val);
ASTNode* create_var(char *name);
ASTNode* create_assign(char *varName, ASTNode *expr);
ASTNode* create_bin_op(char *op, ASTNode *left, ASTNode *right);
ASTNode* create_if(ASTNode *cond, ASTNode *thenStmt, ASTNode *elseStmt);
ASTNode* create_while(ASTNode *cond, ASTNode *body);
ASTNode* create_seq(ASTNode *stmt1, ASTNode *stmt2);
ASTNode* create_print(ASTNode *args);
ASTNode* create_read(char *varName, int type);
ASTNode* create_float_const(float val);
ASTNode* create_array_access(char *name, ASTNode *idx1, ASTNode *idx2);
ASTNode* create_read_array(char *varName, ASTNode *index, int type);
ASTNode* create_read_matrix(char *varName, ASTNode *row, ASTNode *col, int type);
ASTNode* create_assign_idx(char *name, ASTNode *idx1, ASTNode *idx2, ASTNode *val);
ASTNode* create_unit_def(char *name, ASTNode *fields);
ASTNode* create_access(char *var, char *field);
ASTNode* create_for(char *varName, ASTNode *start, ASTNode *end, ASTNode *body);
ASTNode* create_func_def(char *name, int retType, ASTNode *params, ASTNode *body);
ASTNode* create_func_call(char *name, ASTNode *args);
ASTNode* create_return(ASTNode *expr);
ASTNode* create_param_list(ASTNode *param, ASTNode *next);
ASTNode* create_arg_list(ASTNode *arg, ASTNode *next);
ASTNode* create_decl(char *name, int type, int kind, int size1, int size2);

void print_ast(ASTNode *node, int level);

#endif