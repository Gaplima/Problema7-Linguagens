#include <stdio.h>
#include <string.h>
#include "ast.h"

// Arquivo de saída global para simplificar
FILE *f = NULL;

const char* get_type_name(int type) {
    switch(type) {
        case 0: return "int";       // TYPE_INT (ajuste conforme seu %token)
        case 1: return "float";     // TYPE_FLOAT
        case 2: return "char";      // TYPE_CHAR
        case 3: return "char*";     // TYPE_STRING
        default: return "int";
    }
}

// O Parser usa define 258 para TYPE_INT, etc.
// Mapeie conforme os valores do seu y.tab.h ou parser.
// Assumindo: TYPE_INT=258, ARRAY=259, CHAR=260, STRING=261, FLOAT=262
const char* map_type(int type) {
    // Hack simples: se o número for grande, tentamos mapear.
    // O ideal é incluir y.tab.h aqui, mas vamos usar strings diretas.
    return "int"; // Por padrão, tudo vira int/float em C
}

void gen_code(ASTNode *node) {
    if (!node) return;

    switch (node->type) {
        
        case NODE_SEQ:
            gen_code(node->left);
            gen_code(node->right);
            break;

        case NODE_DECL:
            // Traduz: int x; ou int v[10];
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
            // Traduz: int a, int b
            if (node->left) { // Param Atual
                 ASTNode *p = node->left; // É um NODE_VAR mas precisamos do tipo
                 // Simplificação: Assumindo int para params na visualização
                 fprintf(f, "int %s", p->strValue); 
            }
            if (node->right) {
                fprintf(f, ", ");
                gen_code(node->right);
            }
            break;

        case NODE_BLOCK:
            // Se for o nó raiz de comandos (main implícito), tratamos no parser.
            // Aqui tratamos blocos internos { }
            fprintf(f, "{\n");
            gen_code(node->left);
            fprintf(f, "}\n");
            break;

        case NODE_ASSIGN:
            // x = expr;
            fprintf(f, "%s = ", node->strValue);
            gen_code(node->left);
            fprintf(f, ";\n");
            break;

        case NODE_ASSIGN_IDX:
            // v[i] = expr;
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
            fprintf(f, "%d", node->intValue);
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
            // Pascal: for i := start to end
            // C: for (i = start; i <= end; i++)
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
        case NODE_PRINT:
            {
                ASTNode *arg = node->left;
                
                // O argumento pode ser uma lista (NODE_ARG_LIST) ou um item único
                while (arg != NULL) {
                    // Escreve a chamada formatada corretamente para C
                    fprintf(f, "printf(\"%%d\\n\", ");
                    
                    if (arg->type == NODE_ARG_LIST) {
                        gen_code(arg->left); // Gera o valor (ex: numeros[i])
                        arg = arg->right;    // Avança para o próximo
                    } else {
                        gen_code(arg);       // Gera o valor único
                        arg = NULL;          // Encerra
                    }
                    
                    fprintf(f, ");\n"); // Fecha o printf e adiciona o ponto e vírgula!
                }
            }
            break;
    }
}

/* codegen.c - Substitua a função generate_c_code antiga por esta */

void generate_c_code(ASTNode *root, char *input_filename) {
    char output_filename[256];
    
    // Copia o nome de entrada
    strncpy(output_filename, input_filename, 250);
    
    // Encontra o ponto da extensão
    char *ext = strrchr(output_filename, '.');
    if (ext != NULL) {
        // Substitui a extensão original por .c
        strcpy(ext, ".c");
    } else {
        // Se não tiver extensão, apenas adiciona .c
        strcat(output_filename, ".c");
    }

    f = fopen(output_filename, "w");
    if (!f) {
        printf("Erro ao criar arquivo de saida: %s\n", output_filename);
        return;
    }

    fprintf(f, "#include <stdio.h>\n");
    fprintf(f, "\n// Codigo gerado pelo compilador\n\n");

    // Lógica de geração (igual à anterior)
    if (root->type == NODE_SEQ) {
        gen_code(root->left); 
        fprintf(f, "\nint main() {\n");
        if (root->right && root->right->type == NODE_BLOCK) {
             gen_code(root->right->left);
        } else {
             gen_code(root->right);
        }
        fprintf(f, "\nreturn 0;\n");
        fprintf(f, "}\n");
    } else {
        fprintf(f, "int main() {\n");
        gen_code(root);
        fprintf(f, "return 0;\n}\n");
    }

    fclose(f);
    printf("Compilacao concluida! Gerado: '%s'\n", output_filename);
}