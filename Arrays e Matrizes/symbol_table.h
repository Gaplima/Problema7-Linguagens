// symbol_table.h
#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TABLE_SIZE 101 

// Tipos de estrutura de dados
#define KIND_SCALAR 0
#define KIND_ARRAY  1
#define KIND_MATRIX 2

typedef struct Symbol {
    char *name;
    int type;           // TYPE_INT, TYPE_FLOAT...
    int kind;           // NOVO: SCALAR, ARRAY ou MATRIX
    int size1;          // NOVO: Tamanho dimensão 1
    int size2;          // NOVO: Tamanho dimensão 2
    struct Symbol *next;
} Symbol;

extern Symbol* symbolTable[TABLE_SIZE];

unsigned int hash(char *s);
void init_symbol_table();
Symbol* lookup_symbol(char *name);
// Assinatura atualizada
void install_symbol(char *name, int type, int kind, int size1, int size2);
void print_symbol_table();

#endif