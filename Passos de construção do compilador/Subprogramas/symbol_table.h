#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TABLE_SIZE 101 
#define KIND_SCALAR 0
#define KIND_ARRAY  1
#define KIND_MATRIX 2
#define KIND_FUNCTION 3  // <--- Para identificar nomes de função

typedef struct Symbol {
    char *name;
    int type;           
    int kind;           
    int size1;          
    int size2;
    int scope;          // <--- Nível do escopo (0=Global, 1=Local)
    struct Symbol *next;
} Symbol;

extern Symbol* symbolTable[TABLE_SIZE];
extern int current_scope; // Variável global para controlar onde estamos

unsigned int hash(char *s);
void init_symbol_table();

// Funções de Escopo
void enter_scope();
void exit_scope();

Symbol* lookup_symbol(char *name);
void install_symbol(char *name, int type, int kind, int size1, int size2);
void print_symbol_table();

#endif