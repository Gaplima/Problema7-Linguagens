#include "ast.h"
#include "y.tab.h"

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
    node->left = start;
    node->right = end;
    node->extra = body;
    return node;
}

ASTNode* create_decl(char *name, int type, int kind, int size1, int size2) {
    ASTNode *node = create_node(NODE_DECL);
    node->strValue = strdup(name);
    node->dataType = type;
    node->kind = kind;
    node->size1 = size1;
    node->size2 = size2;
    return node;
}

ASTNode* create_seq(ASTNode *stmt1, ASTNode *stmt2) {
    ASTNode *node = create_node(NODE_SEQ);
    node->left = stmt1;
    node->right = stmt2;
    return node;
}

ASTNode* create_print(ASTNode *args) {
    ASTNode *node = create_node(NODE_PRINT);
    node->left = args; 
    return node;
}

ASTNode* create_read(char *varName, int type) {
    ASTNode *node = create_node(NODE_READ);
    node->strValue = strdup(varName); // Guarda o nome da variável
    node->dataType = type;            // Guarda o tipo (INT, FLOAT, STRING)
    return node;
}

ASTNode* create_float_const(float val) {
    ASTNode *node = create_node(NODE_CONST);
    node->floatValue = val;
    node->dataType = 290; // (Ou use TYPE_FLOAT se tiver acesso ao header)
    return node;
}

ASTNode* create_array_access(char *name, ASTNode *idx1, ASTNode *idx2) {
    ASTNode *node = create_node(NODE_ARRAY_ACCESS);
    node->strValue = strdup(name);
    node->left = idx1;
    node->right = idx2;
    return node;
}

ASTNode* create_assign_idx(char *name, ASTNode *idx1, ASTNode *idx2, ASTNode *val) {
    ASTNode *node = create_node(NODE_ASSIGN_IDX);
    node->strValue = strdup(name);
    node->left = idx1;
    node->right = idx2;
    node->extra = val;
    return node;
}

ASTNode* create_func_def(char *name, int retType, ASTNode *params, ASTNode *body) {
    ASTNode *node = create_node(NODE_FUNC_DEF);
    node->strValue = strdup(name);
    node->dataType = retType;
    node->left = params;
    node->right = body;
    return node;
}

ASTNode* create_func_call(char *name, ASTNode *args) {
    ASTNode *node = create_node(NODE_FUNC_CALL);
    node->strValue = strdup(name);
    node->left = args;
    return node;
}

ASTNode* create_return(ASTNode *expr) {
    ASTNode *node = create_node(NODE_RETURN);
    node->left = expr;
    return node;
}

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
            break; // <--- CORREÇÃO: ADICIONADO BREAK QUE FALTAVA
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