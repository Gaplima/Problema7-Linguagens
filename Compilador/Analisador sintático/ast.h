// ast.h
#ifndef AST_H
#define AST_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Tipos de nós possíveis na árvore
typedef enum {
    NODE_CONST,
    NODE_VAR,
    NODE_ASSIGN,
    NODE_IF,
    NODE_WHILE,
    NODE_BLOCK,
    NODE_BIN_OP, // Operações binárias (+, -, *, <, ==, etc)
    NODE_SEQ     // Sequência de comandos (para stmt_list)
} NodeType;

// Estrutura do Nó da Árvore
typedef struct ASTNode {
    NodeType type;
    
    // Dados para folhas
    int intValue;       // Para números
    char *strValue;     // Para identificadores ou operadores (+, -, *)

    // Filhos
    struct ASTNode *left;
    struct ASTNode *right;
    struct ASTNode *extra; // Útil para o 'ELSE' no IF
} ASTNode;

// Protótipos das funções de criação
ASTNode* create_node(NodeType type);
ASTNode* create_const(int val);
ASTNode* create_var(char *name);
ASTNode* create_assign(char *varName, ASTNode *expr);
ASTNode* create_bin_op(char *op, ASTNode *left, ASTNode *right);
ASTNode* create_if(ASTNode *cond, ASTNode *thenStmt, ASTNode *elseStmt);
ASTNode* create_while(ASTNode *cond, ASTNode *body);
ASTNode* create_seq(ASTNode *stmt1, ASTNode *stmt2); // Para listas de comandos

// Função para imprimir a árvore
void print_ast(ASTNode *node, int level);

#endif