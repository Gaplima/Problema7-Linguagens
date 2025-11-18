// symbol_table.h
#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define o tamanho da Hash Table (número primo é melhor para distribuição)
#define TABLE_SIZE 101 

// Estrutura de um Símbolo
typedef struct Symbol {
    char *name;         // Nome da variável (ex: "x", "contador")
    int type;           // Tipo da variável (ex: TYPE_INT, TYPE_FLOAT) - vindo do lexer/parser
    struct Symbol *next;// Próximo símbolo em caso de colisão (Chaining)
} Symbol;

// A Tabela é um array de ponteiros para Symbols
extern Symbol* symbolTable[TABLE_SIZE];

// Protótipos
unsigned int hash(char *s);
void init_symbol_table();
Symbol* lookup_symbol(char *name);
void install_symbol(char *name, int type);
void print_symbol_table();

#endif