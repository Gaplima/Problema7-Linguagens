#include <stdio.h>
#include <string.h>
#include "ast.h"
#include "y.tab.h"

// Arquivo de saída global para simplificar
FILE *f = NULL;

const char* get_type_name(int type) {
    if (type == TYPE_INT) return "int";
    if (type == TYPE_FLOAT) return "float";
    if (type == TYPE_CHAR) return "char";
    if (type == TYPE_STRING) return "char*"; // Em C, string é char*
    return "void";
}

const char* map_type(int type) {
    switch (type) {
        case TYPE_INT:      return "int";
        case TYPE_FLOAT:    return "float";
        case TYPE_CHAR:     return "char";
        case TYPE_STRING:   return "char*"; // String em C é ponteiro de char
        default:            return "int";   // Fallback seguro
    }
}

void gen_code(ASTNode *node) {
    if (!node) return;

    switch (node->type) {
        
        case NODE_SEQ:
            gen_code(node->left);
            gen_code(node->right);
            break;

        case NODE_DECL:
            // Traduz: int x; ou char* s;
            fprintf(f, "%s %s", map_type(node->dataType), node->strValue);
            
            if (node->kind == 1) { // Array
                fprintf(f, "[%d]", node->size1);
            } else if (node->kind == 2) { // Matriz
                fprintf(f, "[%d][%d]", node->size1, node->size2);
            }
            fprintf(f, ";\n");
            break;

        case NODE_FUNC_DEF:
            // Traduz: int nome(params) { body }
            fprintf(f, "\n%s %s(", map_type(node->dataType), node->strValue);
            gen_code(node->left); // Params
            fprintf(f, ") {\n");
            gen_code(node->right); // Body
            fprintf(f, "}\n");
            break;

        case NODE_PARAM_LIST:
            // Traduz params. Ex: int a, char* b
            if (node->left) { 
                 ASTNode *p = node->left; 
                 // Assume-se que o dataType foi preenchido corretamente no parser
                 fprintf(f, "%s %s", map_type(p->dataType), p->strValue); 
            }
            if (node->right) {
                fprintf(f, ", ");
                gen_code(node->right);
            }
            break;

        case NODE_BLOCK:
            fprintf(f, "{\n");
            gen_code(node->left);
            fprintf(f, "}\n");
            break;

        case NODE_ASSIGN:
            fprintf(f, "%s = ", node->strValue);
            gen_code(node->left);
            fprintf(f, ";\n");
            break;

        case NODE_ASSIGN_IDX:
            fprintf(f, "%s[", node->strValue);
            gen_code(node->left); // Index 1
            fprintf(f, "]");
            if (node->right) { // Index 2
                fprintf(f, "[");
                gen_code(node->right);
                fprintf(f, "]");
            }
            fprintf(f, " = ");
            gen_code(node->extra); // Valor
            fprintf(f, ";\n");
            break;

        case NODE_VAR:
            fprintf(f, "%s", node->strValue);
            break;

        case NODE_CONST:
            if (node->dataType == TYPE_STRING) {
                fprintf(f, "%s", node->strValue);
            } 
            else if (node->dataType == TYPE_FLOAT) { /* <--- NOVA VERIFICAÇÃO */
                fprintf(f, "%f", node->floatValue);
            } 
            else {
                fprintf(f, "%d", node->intValue);
            }
            break;

        case NODE_BIN_OP:
            fprintf(f, "(");
            gen_code(node->left);
            fprintf(f, " %s ", node->strValue);
            gen_code(node->right);
            fprintf(f, ")");
            break;

        case NODE_IF:
            fprintf(f, "if (");
            gen_code(node->left);
            fprintf(f, ") {\n");
            gen_code(node->right);
            fprintf(f, "}\n");
            if (node->extra) {
                fprintf(f, "else {\n");
                gen_code(node->extra);
                fprintf(f, "}\n");
            }
            break;

        case NODE_WHILE:
            fprintf(f, "while (");
            gen_code(node->left);
            fprintf(f, ") {\n");
            gen_code(node->right);
            fprintf(f, "}\n");
            break;

        case NODE_FOR:
            fprintf(f, "for (%s = ", node->strValue);
            gen_code(node->left); // Start
            fprintf(f, "; %s <= ", node->strValue);
            gen_code(node->right); // End
            fprintf(f, "; %s++) {\n", node->strValue);
            gen_code(node->extra); // Body
            fprintf(f, "}\n");
            break;

        case NODE_RETURN:
            fprintf(f, "return ");
            gen_code(node->left);
            fprintf(f, ";\n");
            break;

        case NODE_FUNC_CALL:
            fprintf(f, "%s(", node->strValue);
            gen_code(node->left); // Args
            fprintf(f, ")");
            break;

        case NODE_ARG_LIST:
            gen_code(node->left);
            if (node->right) {
                fprintf(f, ", ");
                gen_code(node->right);
            }
            break;

        case NODE_ARRAY_ACCESS:
            fprintf(f, "%s[", node->strValue);
            gen_code(node->left);
            fprintf(f, "]");
            if (node->right) {
                fprintf(f, "[");
                gen_code(node->right);
                fprintf(f, "]");
            }
            break;

        case NODE_READ:
            if (node->dataType == TYPE_INT) {
                fprintf(f, "scanf(\"%%d\", &%s);\n", node->strValue);
            } 
            else if (node->dataType == TYPE_FLOAT) {
                fprintf(f, "scanf(\"%%f\", &%s);\n", node->strValue);
            }
            else if (node->dataType == TYPE_STRING) {
                // Aloca memória temporária para a string (segurança básica)
                // scanf("%s") lê até o primeiro espaço. Para ler linha toda seria gets/fgets.
                // Vamos usar %s simples por enquanto.
                fprintf(f, "%s = (char*)malloc(256 * sizeof(char));\n", node->strValue);
                fprintf(f, "scanf(\"%%255s\", %s);\n", node->strValue); // Sem & para char*
            }
            break;

        case NODE_PRINT: {
            ASTNode *arg = node->left;
            while (arg != NULL) {
                // Pega o argumento atual (pode estar numa lista ou ser unico)
                ASTNode *val = (arg->type == NODE_ARG_LIST) ? arg->left : arg;
                
                // Verifica o tipo para usar o format specifier correto
                if (val->dataType == TYPE_STRING) {
                    fprintf(f, "printf(\"%%s\\n\", ");
                    
                    if (val->type == NODE_CONST) {
                         // É literal string (ex: "Ola")
                         fprintf(f, "%s", val->strValue);
                    } else {
                         // É variavel string (ex: msg)
                         gen_code(val);
                    }
                } else if (val->dataType == TYPE_FLOAT) {
                    fprintf(f, "printf(\"%%f\\n\", ");
                    gen_code(val);
                } else {
                    // Padrão Int/Char
                    fprintf(f, "printf(\"%%d\\n\", ");
                    gen_code(val);
                }
                
                fprintf(f, ");\n");
                
                // Avança para o próximo argumento
                if (arg->type == NODE_ARG_LIST) arg = arg->right;
                else arg = NULL;
            }
            break;
        }
    }
}

void generate_c_code(ASTNode *root, char *input_filename) {
    char output_filename[256];
    
    // Gera nome do arquivo de saida (entrada.txt -> entrada.c)
    strncpy(output_filename, input_filename, 250);
    char *ext = strrchr(output_filename, '.');
    if (ext != NULL) {
        strcpy(ext, ".c");
    } else {
        strcat(output_filename, ".c");
    }

    f = fopen(output_filename, "w");
    if (!f) {
        printf("Erro ao criar arquivo de saida: %s\n", output_filename);
        return;
    }

    fprintf(f, "#include <stdio.h>\n");
    fprintf(f, "#include <stdlib.h>\n");
    fprintf(f, "\n// Codigo gerado pelo compilador\n\n");

    if (root->type == NODE_SEQ) {
        // Gera globais primeiro (lado esquerdo da sequencia raiz)
        gen_code(root->left); 
        
        fprintf(f, "\nint main() {\n");
        
        // Gera o bloco principal (lado direito)
        if (root->right && root->right->type == NODE_BLOCK) {
             gen_code(root->right->left); // Pula o wrapper BLOCK_BEGIN
        } else {
             gen_code(root->right);
        }
        
        fprintf(f, "\nreturn 0;\n");
        fprintf(f, "}\n");
    } else {
        // Caso programa muito simples (apenas main)
        fprintf(f, "int main() {\n");
        gen_code(root);
        fprintf(f, "return 0;\n}\n");
    }

    fclose(f);
    printf("Compilacao concluida! Gerado: '%s'\n", output_filename);
}