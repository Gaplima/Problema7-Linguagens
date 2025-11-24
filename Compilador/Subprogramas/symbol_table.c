// symbol_table.c
#include "symbol_table.h"

// Declaração global da tabela
Symbol* symbolTable[TABLE_SIZE];

int current_scope = 0; // Começa no escopo Global

// Função de Hash (djb2 by Dan Bernstein)
unsigned int hash(char *s) {
    unsigned int hash = 5381;
    int c;
    while ((c = *s++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash % TABLE_SIZE;
}

// Inicializa a tabela com NULL
void init_symbol_table() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        symbolTable[i] = NULL;
    }
}

// Aumenta o nível (entra na função)
void enter_scope() {
    current_scope++;
}

// Diminui o nível e LIMPA as variáveis locais da tabela
void exit_scope() {
    // Percorre toda a tabela hash removendo símbolos do escopo atual
    for (int i = 0; i < TABLE_SIZE; i++) {
        Symbol *prev = NULL;
        Symbol *curr = symbolTable[i];
        
        while (curr != NULL) {
            if (curr->scope == current_scope) {
                // Remove este símbolo
                Symbol *toFree = curr;
                if (prev == NULL) {
                    symbolTable[i] = curr->next;
                    curr = symbolTable[i];
                } else {
                    prev->next = curr->next;
                    curr = prev->next;
                }
                free(toFree->name);
                free(toFree);
            } else {
                // Avança
                prev = curr;
                curr = curr->next;
            }
        }
    }
    current_scope--;
}

// Busca um símbolo na tabela
Symbol* lookup_symbol(char *name) {
    unsigned int idx = hash(name);
    Symbol *sym = symbolTable[idx];
    Symbol *bestMatch = NULL; // Precisamos achar o do escopo mais "perto"
    
    while (sym != NULL) {
        if (strcmp(sym->name, name) == 0) {
            // Se achou um símbolo com mesmo nome
            // Preferência: O que tiver o escopo maior (mais local)
            // Mas não podemos pegar escopo maior que o atual (futuro/inválido)
            if (sym->scope <= current_scope) {
                if (bestMatch == NULL || sym->scope > bestMatch->scope) {
                    bestMatch = sym;
                }
            }
        }
        sym = sym->next;
    }
    return bestMatch;
}

// Insere um símbolo novo na tabela
// symbol_table.c (apenas a função install_symbol e print mudam)
void install_symbol(char *name, int type, int kind, int size1, int size2) {
    unsigned int idx = hash(name);
    Symbol *newSym = (Symbol*) malloc(sizeof(Symbol));
    newSym->name = strdup(name);
    newSym->type = type;
    newSym->kind = kind;     // Gravando novos metadados
    newSym->size1 = size1;
    newSym->size2 = size2;
    newSym->scope = current_scope; // <--- Grava o escopo atual (0 ou 1)
    
    newSym->next = symbolTable[idx];
    symbolTable[idx] = newSym;
    
    if (kind == KIND_FUNCTION) {
         printf("[DEBUG] Funcao '%s' declarada (Global).\n", name);
    } else {
         printf("[DEBUG] Var '%s' instalada no Escopo %d.\n", name, current_scope);
    }
    
    if(kind == KIND_SCALAR) 
        printf("[DEBUG] Var '%s' instalada.\n", name);
    else if(kind == KIND_ARRAY)
        printf("[DEBUG] Array '%s'[%d] instalado.\n", name, size1);
    else
        printf("[DEBUG] Matriz '%s'[%d][%d] instalada.\n", name, size1, size2);
}

// Função auxiliar para debug visual
void print_symbol_table() {
    printf("\n--- Tabela de Simbolos ---\n");
    for (int i = 0; i < TABLE_SIZE; i++) {
        Symbol *sym = symbolTable[i];
        if (sym != NULL) {
            printf("[%d]: ", i);
            while (sym != NULL) {
                printf("%s -> ", sym->name);
                sym = sym->next;
            }
            printf("NULL\n");
        }
    }
    printf("--------------------------\n");
}