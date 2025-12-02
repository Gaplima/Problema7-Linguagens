// symbol_table.c
#include "symbol_table.h"

Symbol* symbolTable[TABLE_SIZE];

int current_scope = 0; 

unsigned int hash(char *s) {
    unsigned int hash = 5381;
    int c;
    while ((c = *s++))
        hash = ((hash << 5) + hash) + c; 
    return hash % TABLE_SIZE;
}

void init_symbol_table() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        symbolTable[i] = NULL;
    }
}

void enter_scope() {
    current_scope++;
}

void exit_scope() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        Symbol *prev = NULL;
        Symbol *curr = symbolTable[i];
        
        while (curr != NULL) {
            if (curr->scope == current_scope) {
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
                prev = curr;
                curr = curr->next;
            }
        }
    }
    current_scope--;
}

Symbol* lookup_symbol(char *name) {
    unsigned int idx = hash(name);
    Symbol *sym = symbolTable[idx];
    Symbol *bestMatch = NULL;
    
    while (sym != NULL) {
        if (strcmp(sym->name, name) == 0) {
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

void install_symbol(char *name, int type, int kind, int size1, int size2) {
    unsigned int idx = hash(name);
    Symbol *newSym = (Symbol*) malloc(sizeof(Symbol));
    newSym->name = strdup(name);
    newSym->type = type;
    newSym->kind = kind;
    newSym->size1 = size1;
    newSym->size2 = size2;
    newSym->scope = current_scope;
    
    newSym->next = symbolTable[idx];
    symbolTable[idx] = newSym;
    
    if (kind == KIND_FUNCTION) {
         printf("[DEBUG] Funcao '%s' declarada (Global).\n", name);
    } else if (kind == KIND_SCALAR) {
         printf("[DEBUG] Var '%s' instalada no Escopo %d.\n", name, current_scope);
    } else if (kind == KIND_UNIT) {  /* <--- ADICIONE ESTE BLOCO */
         printf("[DEBUG] Unit/Struct '%s' instalada.\n", name);
    } else if(kind == KIND_ARRAY) {
        printf("[DEBUG] Array '%s'[%d] instalado.\n", name, size1);
    } else {
        printf("[DEBUG] Matriz '%s'[%d][%d] instalada.\n", name, size1, size2);
    }
}

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