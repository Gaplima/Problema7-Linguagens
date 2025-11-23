// symbol_table.c
#include "symbol_table.h"

// Declaração global da tabela
Symbol* symbolTable[TABLE_SIZE];

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

// Busca um símbolo na tabela
Symbol* lookup_symbol(char *name) {
    unsigned int idx = hash(name);
    Symbol *sym = symbolTable[idx];
    
    // Percorre a lista encadeada nessa posição
    while (sym != NULL) {
        if (strcmp(sym->name, name) == 0) {
            return sym; // Encontrou
        }
        sym = sym->next;
    }
    return NULL; // Não encontrou
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
    
    newSym->next = symbolTable[idx];
    symbolTable[idx] = newSym;
    
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