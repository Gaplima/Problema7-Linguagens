#include <stdio.h>
#include <string.h>
#include "ast.h"
#include "y.tab.h"
#include "symbol_table.h"

/* * Variável Global 'f': 
 * Aponta para o ficheiro .c que está a ser gerado. 
 * É global para evitar passá-la como argumento em todas as chamadas recursivas.
 */
FILE *f = NULL;

/*
 * Função Auxiliar: map_type
 * Traduz os tipos internos da linguagem (TYPE_INT, etc.) para os tipos da linguagem C.
 * Exemplo: TYPE_STRING torna-se "char*" (ponteiro de caracteres).
 */
const char* map_type(int type) {
    switch (type) {
        case TYPE_INT:      return "int";
        case TYPE_FLOAT:    return "float";
        case TYPE_CHAR:     return "char";
        case TYPE_STRING:   return "char*"; // String em C é tratada como ponteiro
		case TYPE_ARRAY:    return "int*";
        default:            return "int";   // Fallback de segurança
    }
}

/*
 * FUNÇÃO PRINCIPAL DE GERAÇÃO (CORE)
 * Percorre a AST recursivamente e escreve o código C equivalente.
 */
void gen_code(ASTNode *node) {
    if (!node) return;

    switch (node->type) {
        /* * NODE_SEQ: Sequenciamento
         * A estrutura da AST para listas de comandos é geralmente:
         * SEQ
         * /   \
         * CMD1   SEQ (ou NULL)
         * Percorre a esquerda (comando atual) e depois a direita (próximos).
         */
        case NODE_SEQ:
            gen_code(node->left);
            gen_code(node->right);
            break;

        /* * NODE_UNIT_DEF: Definição de Estruturas
         * Traduz a palavra-chave 'unit' da sua linguagem para 'struct' em C.
         */
        case NODE_UNIT_DEF:
            fprintf(f, "struct %s {\n", node->strValue);
            gen_code(node->left); /* Gera as declarações dos campos internos */
            fprintf(f, "};\n");
            break;

        /* * NODE_DECL: Declaração de Variáveis
         * Trata casos especiais como Strings e Arrays.
         */
        case NODE_DECL:
             if (node->kind == KIND_UNIT) {
                /* Declaração de instância de struct: struct Ponto p; */
                fprintf(f, "struct %s %s;\n", node->unitName, node->strValue);
            } else if (node->dataType == TYPE_STRING && node->kind == KIND_SCALAR) {
				/* Nota:
                 * Em C, declarar 'char *s;' não aloca memória para o texto.
                 * Aqui, forçamos 'char s[256];' para garantir espaço buffer.
                 */
				fprintf(f, "char %s[256];\n", node->strValue);
			}
			else {
                /* Declaração Padrão (int, float...) */
                fprintf(f, "%s %s", map_type(node->dataType), node->strValue);
                
                /* Adiciona dimensões se for Array ou Matriz */
                if (node->kind == KIND_ARRAY) {
                    fprintf(f, "[%d]", node->size1);
                } else if (node->kind == KIND_MATRIX) {
                    fprintf(f, "[%d][%d]", node->size1, node->size2);
                }
                fprintf(f, ";\n");
            } 
            break; 

        /* Acesso a campos: p1.x */
        case NODE_ACCESS:
            fprintf(f, "%s.%s", node->strValue, node->extra->strValue);
            break;

        /* * NODE_PARAM_LIST: Lista de Parâmetros de Função
         * A recursão aqui é invertida ou ajustada para garantir a ordem correta das vírgulas.
         */
        case NODE_PARAM_LIST:
			if (node->right) {
				gen_code(node->right);
				fprintf(f, ", ");
			}
			if (node->left) { 
				 ASTNode *p = node->left; 
				 /* Se for Unit (struct), escreve "struct Nome var" */
				 if (p->dataType == 1000 && p->unitName != NULL) {
					 fprintf(f, "struct %s %s", p->unitName, p->strValue);
				 } else {
					 /* Caso contrario, usa o tipo primitivo */
					 fprintf(f, "%s %s", map_type(p->dataType), p->strValue); 
				 }
			}
			break;

        /* Blocos de código delimitados por chaves {} */
        case NODE_BLOCK:
            fprintf(f, "{\n");
            gen_code(node->left);
            fprintf(f, "}\n");
            break;

        /* * NODE_ASSIGN: Atribuição (=)
         * Verifica se é uma atribuição normal ou em um campo de struct.
         */
        case NODE_ASSIGN:
            if (node->left && node->left->type == NODE_ACCESS) {
                 gen_code(node->left); // Gera o lado esquerdo (ex: p1.x)
                 fprintf(f, " = ");
                 gen_code(node->right);
                 fprintf(f, ";\n");
            } else {
                 fprintf(f, "%s = ", node->strValue);
                 gen_code(node->left);
                 fprintf(f, ";\n");
            }
            break;

        /* Atribuição em Arrays/Matrizes: v[0] = 10 */
        case NODE_ASSIGN_IDX:
            fprintf(f, "%s[", node->strValue);
            gen_code(node->left); // Índice 1
            fprintf(f, "]");
            if (node->right) { // Índice 2 (se for matriz)
                fprintf(f, "[");
                gen_code(node->right);
                fprintf(f, "]");
            }
            fprintf(f, " = ");
            gen_code(node->extra); // Valor a atribuir
            fprintf(f, ";\n");
            break;

        /* Uso de Variável simples */
        case NODE_VAR:
            fprintf(f, "%s", node->strValue);
            break;

        /* Literais (Números ou Strings fixas no código) */
        case NODE_CONST:
            if (node->dataType == TYPE_STRING) {
                fprintf(f, "%s", node->strValue); // Já vem com aspas do Lexer normalmente
            } 
            else if (node->dataType == TYPE_FLOAT) {
                fprintf(f, "%f", node->floatValue);
            } 
            else {
                fprintf(f, "%d", node->intValue);
            }
            break;

        /* * NODE_BIN_OP: Operações Matemáticas/Lógicas
         * Traduz operadores infixos.
         * Nota: A potência '^' não existe em C, então convertemos para a função 'pow()'.
         */
        case NODE_BIN_OP:
            if (strcmp(node->strValue, "^") == 0) {
                fprintf(f, "pow(");
                gen_code(node->left);
                fprintf(f, ", ");
                gen_code(node->right);
                fprintf(f, ")");
            } 
            else {
                /* Padrão: (A + B) */
                fprintf(f, "(");
                gen_code(node->left);
                fprintf(f, " %s ", node->strValue);
                gen_code(node->right);
                fprintf(f, ")");
            }
            break;

        /* Estruturas de Controlo (IF, WHILE, FOR) - Tradução direta para C */
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
            gen_code(node->left); // Valor Inicial
            fprintf(f, "; %s <= ", node->strValue);
            gen_code(node->right); // Condição de paragem
            fprintf(f, "; %s++) {\n", node->strValue);
            gen_code(node->extra); // Corpo do loop
            fprintf(f, "}\n");
            break;
		
		/* Gera: goto label; */
        case NODE_GOTO:
            fprintf(f, "goto %s;\n", node->strValue);
            break;

        /* Gera: label: */
        case NODE_LABEL:
            /* Nota: Em C, um label não pode ser a última coisa de um bloco.
             * Adicionamos um ';' vazio por segurança (null statement). 
             */
            fprintf(f, "%s:\n;\n", node->strValue);
            break;
		
        case NODE_RETURN:
            fprintf(f, "return ");
            gen_code(node->left);
            fprintf(f, ";\n");
            break;
        
        /* Definição de Funções */
        case NODE_FUNC_DEF:
            /* Verifica se o retorno é uma Struct (unitName não nulo) */
            if (node->dataType == 1000 && node->unitName != NULL) {
                fprintf(f, "\nstruct %s %s(", node->unitName, node->strValue);
            } else {
                /* Retorno primitivo (int, float, etc) */
                fprintf(f, "\n%s %s(", map_type(node->dataType), node->strValue);
            }
            
            gen_code(node->left); // Gera os parâmetros
            fprintf(f, ") {\n");
            gen_code(node->right); // Gera o corpo
            fprintf(f, "}\n");
            break;
        
        case NODE_FUNC_CALL:
            fprintf(f, "%s(", node->strValue);
            gen_code(node->left); // Argumentos
            fprintf(f, ")");
            break;
        
        /* Cast Explícito gerado pelo Parser (ex: int para float) */
        case NODE_CAST:
            fprintf(f, "(%s)", map_type(node->dataType)); 
            fprintf(f, "("); 
            gen_code(node->left);
            fprintf(f, ")");
            break;    
        
        case NODE_ARG_LIST:
            if (node->right) {
                gen_code(node->right);
                fprintf(f, ", ");
            }
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
            fprintf(f, ");\n"); 
            break;

        /* * NODE_READ: Comando de Leitura (scanf)
         * O scanf precisa de:
         * 1. Um format specifier (%d, %f, %s)
         * 2. O endereço da variável (&var), exceto se for string (que já é ponteiro).
         */
        case NODE_READ: {
            char *fmt = "%d";
            if (node->dataType == TYPE_FLOAT) fmt = "%f";
            else if (node->dataType == TYPE_STRING) fmt = "%255s"; // Limite de segurança
            
            /* Lógica do '&': Inteiros e Floats precisam, Strings/Arrays não */
            if (node->dataType == TYPE_STRING) {
				fprintf(f, "scanf(\"%s\", %s", fmt, node->strValue);
			} else {
				fprintf(f, "scanf(\"%s\", &%s", fmt, node->strValue);
			}
            
            /* Adiciona índices de Array/Matriz se necessário */
            if (node->kind == KIND_ARRAY) {
                fprintf(f, "[");
                gen_code(node->left);
                fprintf(f, "]");
            } 
            else if (node->kind == KIND_MATRIX) {
                fprintf(f, "[");
                gen_code(node->left); 
                fprintf(f, "][");
                gen_code(node->right); 
                fprintf(f, "]");
            }

            fprintf(f, ");\n");
            break;
        }

        /* * NODE_PRINT: Comando de Escrita (printf)
         * Itera sobre a lista de argumentos para imprimir.
         */
        case NODE_PRINT: {
            ASTNode *arg = node->left;
            while (arg != NULL) {
                ASTNode *val = (arg->type == NODE_ARG_LIST) ? arg->left : arg;
                
                /* Seleciona o printf correto baseado no tipo da expressão */
                if (val->dataType == TYPE_STRING) {
                    fprintf(f, "printf(\"%%s\\n\", ");
                    
                    if (val->type == NODE_CONST) {
                         fprintf(f, "%s", val->strValue);
                    } else {
                         gen_code(val);
                    }
                } else if (val->dataType == TYPE_FLOAT) {
                    fprintf(f, "printf(\"%%f\\n\", ");
                    gen_code(val);
                } else {
                    fprintf(f, "printf(\"%%d\\n\", ");
                    gen_code(val);
                }
                
                fprintf(f, ");\n");
                
                if (arg->type == NODE_ARG_LIST) arg = arg->right;
                else arg = NULL;
            }
            break;
        }
    }
}

/*
 * ==========================================
 * DRIVER: generate_c_code
 * ==========================================
 * Prepara o ficheiro de saída e cria a estrutura básica do programa C (main).
 */
void generate_c_code(ASTNode *root, char *input_filename) {
    char output_filename[256];
    
    /* 1. Manipulação de Strings para mudar extensão .txt/.lan para .c */
    strncpy(output_filename, input_filename, 250);
    char *ext = strrchr(output_filename, '.');
    if (ext != NULL) {
        strcpy(ext, ".c");
    } else {
        strcat(output_filename, ".c");
    }

    /* 2. Abre o ficheiro para escrita ("w") */
    f = fopen(output_filename, "w");
    if (!f) {
        printf("Erro ao criar arquivo de saida: %s\n", output_filename);
        return;
    }

    /* 3. Escreve os Cabeçalhos (Headers) necessários */
    fprintf(f, "#include <stdio.h>\n");
    fprintf(f, "#include <stdlib.h>\n");
    fprintf(f, "#include <math.h>\n");
	fprintf(f, "#include <string.h>\n");
    fprintf(f, "\n// Codigo gerado pelo compilador\n\n");

    /* 4. Gera o corpo do programa */
    if (root->type == NODE_SEQ) {
        /*
         * O Parser organiza a raiz assim:
         * Left: Declarações Globais e Funções
         * Right: Bloco Main
         */
        gen_code(root->left); 
        
        fprintf(f, "\nint main() {\n");
        
        /* Gera o código dentro do main */
        if (root->right && root->right->type == NODE_BLOCK) {
             gen_code(root->right->left); // Pula o nó BLOCK para evitar chaves duplas desnecessárias
        } else {
             gen_code(root->right);
        }
        
        fprintf(f, "\nreturn 0;\n");
        fprintf(f, "}\n");
    } else {
        /* Caso simples: apenas main */
        fprintf(f, "int main() {\n");
        gen_code(root);
        fprintf(f, "return 0;\n}\n");
    }

    fclose(f);
    printf("Compilacao concluida! Gerado: '%s'\n", output_filename);
}