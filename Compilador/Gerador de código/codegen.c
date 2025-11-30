#include <stdio.h>
#include <string.h>
#include "ast.h"
#include "y.tab.h"
#include "symbol_table.h"

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

        case NODE_UNIT_DEF:
            /* Traduz 'unit Nome' para 'struct Nome' */
            fprintf(f, "struct %s {\n", node->strValue);
            gen_code(node->left); 
            fprintf(f, "};\n");
            break;

        case NODE_DECL:
             if (node->kind == KIND_UNIT) {
                fprintf(f, "struct %s %s;\n", node->unitName, node->strValue);
            } else {
                /* IMPRESSÃO CORRETA */
                fprintf(f, "%s %s", map_type(node->dataType), node->strValue);
                
                if (node->kind == KIND_ARRAY) {
                    fprintf(f, "[%d]", node->size1);
                } else if (node->kind == KIND_MATRIX) {
                    fprintf(f, "[%d][%d]", node->size1, node->size2);
                }
                fprintf(f, ";\n");
            } 
            break; 

        case NODE_ACCESS:
            /* p1.x */
            fprintf(f, "%s.%s", node->strValue, node->extra->strValue);
            break;

        case NODE_PARAM_LIST:
            // Trocar a ordem: Imprime o resto da lista (Right) PRIMEIRO
            if (node->right) {
                gen_code(node->right);
                fprintf(f, ", ");
            }
            // Depois imprime o parametro atual (Left)
            if (node->left) { 
                 ASTNode *p = node->left; 
                 fprintf(f, "%s %s", map_type(p->dataType), p->strValue); 
            }
            break;

        case NODE_BLOCK:
            fprintf(f, "{\n");
            gen_code(node->left);
            fprintf(f, "}\n");
            break;

        case NODE_ASSIGN:
            /* Verifica se é uma atribuição especial de acesso */
            if (node->left && node->left->type == NODE_ACCESS) {
                 gen_code(node->left); // Imprime p1.x
                 fprintf(f, " = ");
                 gen_code(node->right);
                 fprintf(f, ";\n");
            } else {
                 // Atribuição normal: x = ...
                 fprintf(f, "%s = ", node->strValue);
                 gen_code(node->left);
                 fprintf(f, ";\n");
            }
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
            /* Verifica se é Potência */
            if (strcmp(node->strValue, "^") == 0) {
                fprintf(f, "pow(");
                gen_code(node->left);
                fprintf(f, ", ");
                gen_code(node->right);
                fprintf(f, ")");
            } 
            else {
                /* Comportamento Padrão (+, -, *, /, &&, ||) */
                fprintf(f, "(");
                gen_code(node->left);
                fprintf(f, " %s ", node->strValue);
                gen_code(node->right);
                fprintf(f, ")");
            }
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
        
        case NODE_FUNC_DEF:
            /* Verifica se é uma função que retorna Struct/Unit */
            if (node->unitName != NULL) {
                fprintf(f, "\nstruct %s %s(", node->unitName, node->strValue);
            } else {
                /* Função normal (int, float...) */
                fprintf(f, "\n%s %s(", map_type(node->dataType), node->strValue);
            }
            
            gen_code(node->left); // Params
            fprintf(f, ") {\n");
            gen_code(node->right); // Body
            fprintf(f, "}\n");
            break;
        
        case NODE_FUNC_CALL:
            fprintf(f, "%s(", node->strValue);
            gen_code(node->left); // Args
            fprintf(f, ")");
            break;

        case NODE_ARG_LIST:
            // Trocar a ordem: Imprime o resto da lista (Right) PRIMEIRO
            if (node->right) {
                gen_code(node->right);
                fprintf(f, ", ");
            }
            // Depois imprime o argumento atual (Left)
            gen_code(node->left);
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

        case NODE_PROC_CALL:
            fprintf(f, "%s(", node->strValue);
            gen_code(node->left);
            fprintf(f, ");\n"); /* Nota o ponto e vírgula e quebra de linha aqui */
            break;

        case NODE_READ: {
            // 1. Define o especificador de formato (%d ou %f)
            char *fmt = "%d";
            if (node->dataType == TYPE_FLOAT) fmt = "%f";
            else if (node->dataType == TYPE_STRING) fmt = "%255s"; // Simplificado
            
            // 2. Inicia o scanf: scanf("%d", &nome
            fprintf(f, "scanf(\"%s\", &%s", fmt, node->strValue);
            
            // 3. Se tiver indices, imprime eles: [i] ou [i][j]
            if (node->kind == KIND_ARRAY) {
                fprintf(f, "[");
                gen_code(node->left); // Gera o codigo do indice
                fprintf(f, "]");
            } 
            else if (node->kind == KIND_MATRIX) {
                fprintf(f, "[");
                gen_code(node->left); // Linha
                fprintf(f, "][");
                gen_code(node->right); // Coluna
                fprintf(f, "]");
            }

            // 4. Fecha o scanf
            fprintf(f, ");\n");
            
            // Tratamento especial para malloc de string (se necessario)
            if (node->dataType == TYPE_STRING && node->kind == KIND_SCALAR) {
                 // Nota: Para arrays de string seria mais complexo, 
                 // assumindo aqui apenas leitura simples ou numéricos em arrays.
            }
            break;
        }

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
    fprintf(f, "#include <math.h>\n");
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