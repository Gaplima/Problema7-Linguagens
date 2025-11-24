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
    NODE_BLOCK,
    NODE_BIN_OP,
    NODE_SEQ,
    NODE_ARRAY_ACCESS, // Para ler: x[i]
    NODE_ASSIGN_IDX    // Para atribuir: x[i] := 10
} NodeType;

// Estrutura do Nó da Árvore
typedef struct ASTNode {
    NodeType type;      // Tipo estrutural (se é IF, WHILE, etc)
    int dataType;       // NOVO: Tipo de dado (TYPE_INT, TYPE_FLOAT, etc - vindo do Bison)
    
    int intValue;       
    char *strValue;     

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
ASTNode* create_array_access(char *name, ASTNode *idx1, ASTNode *idx2);
ASTNode* create_assign_idx(char *name, ASTNode *idx1, ASTNode *idx2, ASTNode *val);
ASTNode* create_for(char *varName, ASTNode *start, ASTNode *end, ASTNode *body);

void print_ast(ASTNode *node, int level);

#endif