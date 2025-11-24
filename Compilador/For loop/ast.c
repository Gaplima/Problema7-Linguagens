#include "ast.h"

ASTNode* create_node(NodeType type) {
    ASTNode *node = (ASTNode*) malloc(sizeof(ASTNode));
    node->type = type;
    node->left = NULL;
    node->right = NULL;
    node->extra = NULL;
    node->strValue = NULL;
    return node;
}

ASTNode* create_const(int val) {
    ASTNode *node = create_node(NODE_CONST);
    node->intValue = val;
    return node;
}

ASTNode* create_var(char *name) {
    ASTNode *node = create_node(NODE_VAR);
    node->strValue = strdup(name);
    return node;
}

ASTNode* create_assign(char *varName, ASTNode *expr) {
    ASTNode *node = create_node(NODE_ASSIGN);
    node->strValue = strdup(varName);
    node->left = expr;
    return node;
}

ASTNode* create_bin_op(char *op, ASTNode *left, ASTNode *right) {
    ASTNode *node = create_node(NODE_BIN_OP);
    node->strValue = strdup(op);
    node->left = left;
    node->right = right;
    return node;
}

ASTNode* create_if(ASTNode *cond, ASTNode *thenStmt, ASTNode *elseStmt) {
    ASTNode *node = create_node(NODE_IF);
    node->left = cond;
    node->right = thenStmt;
    node->extra = elseStmt;
    return node;
}

ASTNode* create_while(ASTNode *cond, ASTNode *body) {
    ASTNode *node = create_node(NODE_WHILE);
    node->left = cond;
    node->right = body;
    return node;
}

ASTNode* create_for(char *varName, ASTNode *start, ASTNode *end, ASTNode *body) {
    ASTNode *node = create_node(NODE_FOR);
    node->strValue = strdup(varName);
    node->left = start;   // Valor inicial
    node->right = end;    // Valor final
    node->extra = body;   // Bloco de comandos
    return node;
}

ASTNode* create_seq(ASTNode *stmt1, ASTNode *stmt2) {
    ASTNode *node = create_node(NODE_SEQ);
    node->left = stmt1;
    node->right = stmt2;
    return node;
}

// --- NOVAS FUNÇÕES ---

// Cria nó de LEITURA (acesso)
ASTNode* create_array_access(char *name, ASTNode *idx1, ASTNode *idx2) {
    ASTNode *node = create_node(NODE_ARRAY_ACCESS);
    node->strValue = strdup(name);
    node->left = idx1;   // Índice 1
    node->right = idx2;  // Índice 2 (pode ser NULL)
    return node;
}

// Cria nó de ESCRITA (atribuição em índice)
ASTNode* create_assign_idx(char *name, ASTNode *idx1, ASTNode *idx2, ASTNode *val) {
    ASTNode *node = create_node(NODE_ASSIGN_IDX);
    node->strValue = strdup(name);
    node->left = idx1;   // Índice 1
    node->right = idx2;  // Índice 2 (NULL se for vetor)
    node->extra = val;   // Valor a ser guardado
    return node;
}

// --- FIM NOVAS FUNÇÕES ---

void print_indent(int level) {
    for (int i = 0; i < level; i++) printf("  ");
}

void print_ast(ASTNode *node, int level) {
    if (!node) return;

    print_indent(level);

    switch (node->type) {
        case NODE_CONST:
            printf("Num: %d\n", node->intValue);
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
            print_ast(node->left, level);
            if (node->right) print_ast(node->right, level); 
            break;
            
        // --- NOVOS CASES INTEGRADOS DENTRO DO SWITCH ---
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
    }
}