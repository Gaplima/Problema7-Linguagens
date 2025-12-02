#include "symbol_table.h"

/* * A Tabela de Símbolos é implementada como uma Hash Table (Tabela de Dispersão).
 * Isso permite encontrar variáveis pelo nome muito rapidamente (complexidade O(1) média).
 * * Estrutura: Um array de ponteiros para 'Symbol'. Em caso de colisão (dois nomes 
 * gerarem o mesmo índice), usamos uma lista encadeada (chaining).
 */
Symbol* symbolTable[TABLE_SIZE];

/* * Contador de Nível de Escopo:
 * 0 = Global
 * 1 = Dentro de função
 * 2 = Dentro de um if/while/for aninhado, etc.
 */
int current_scope = 0; 

/*
 * Função Hash (DJB2 Algorithm)
 * Transforma uma string (nome da variável) em um número inteiro (índice do array).
 * É uma função famosa por gerar poucas colisões para strings comuns.
 */
unsigned int hash(char *s) {
    unsigned int hash = 5381;
    int c;
    while ((c = *s++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash % TABLE_SIZE;
}

/* Inicializa a tabela limpando todos os ponteiros */
void init_symbol_table() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        symbolTable[i] = NULL;
    }
}

/*
 * Entrar no Escopo:
 * Chamada quando encontramos '{' ou início de função.
 * Apenas incrementa o nível. As variáveis criadas agora terão scope = current_scope.
 */
void enter_scope() {
    current_scope++;
}

/*
 * Sair do Escopo:
 * Chamada quando encontramos '}' ou fim de função.
 * * FUNCIONAMENTO: Varre TODA a tabela hash procurando símbolos que pertencem 
 * ao escopo atual (que está sendo finalizado) e remove-os da memória.
 * Isso garante que variáveis locais deixem de existir fora do seu bloco.
 */
void exit_scope() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        Symbol *prev = NULL;
        Symbol *curr = symbolTable[i];
        
        while (curr != NULL) {
            if (curr->scope == current_scope) {
                /* Achou um símbolo do escopo que está fechando -> Remove */
                Symbol *toFree = curr;
                
                if (prev == NULL) {
                    /* Era o primeiro da lista encadeada */
                    symbolTable[i] = curr->next;
                    curr = symbolTable[i];
                } else {
                    /* Estava no meio da lista */
                    prev->next = curr->next;
                    curr = prev->next;
                }
                
                /* Libera a memória alocada */
                free(toFree->name);
                free(toFree);
            } else {
                /* Símbolo de escopo antigo (global ou anterior) -> Mantém */
                prev = curr;
                curr = curr->next;
            }
        }
    }
    /* Decrementa o nível, voltando para o escopo pai */
    current_scope--;
}

/*
 * Busca de Símbolos (Lookup):
 * Procura um símbolo pelo nome, respeitando as regras de escopo (Shadowing).
 */
Symbol* lookup_symbol(char *name) {
    unsigned int idx = hash(name);
    Symbol *sym = symbolTable[idx];
    Symbol *bestMatch = NULL;
    
    /* Percorre a lista encadeada neste bucket da hash */
    while (sym != NULL) {
        if (strcmp(sym->name, name) == 0) {
            /* Achou o nome. Agora verifica a visibilidade. */
            
            /* Regra: O símbolo deve ter sido declarado num escopo menor ou igual ao atual */
            if (sym->scope <= current_scope) {
                
                /* LÓGICA DE SOMBREAMENTO (Shadowing):
                 * Se eu tenho uma variável global 'x' (scope 0) e uma local 'x' (scope 1),
                 * ambas estão na lista. Eu quero a que tem o MAIOR escopo (a mais local).
                 */
                if (bestMatch == NULL || sym->scope > bestMatch->scope) {
                    bestMatch = sym;
                }
            }
        }
        sym = sym->next;
    }
    return bestMatch;
}

/*
 * Instalação de Símbolos:
 * Cria uma nova entrada na tabela para uma declaração de variável/função.
 */
void install_symbol(char *name, int type, int kind, int size1, int size2) {
    unsigned int idx = hash(name);
    
    /* Aloca o nó do símbolo */
    Symbol *newSym = (Symbol*) malloc(sizeof(Symbol));
    newSym->name = strdup(name);
    newSym->type = type;   /* INT, FLOAT, STRING... */
    newSym->kind = kind;   /* SCALAR, ARRAY, MATRIX, FUNCTION... */
    newSym->size1 = size1; /* Dimensão 1 (para arrays) */
    newSym->size2 = size2; /* Dimensão 2 (para matrizes) */
    newSym->scope = current_scope; /* Marca em qual escopo nasceu */
    
    /* * Inserção na Cabeça (Head Insertion):
     * Insere o novo símbolo no INÍCIO da lista encadeada.
     * Isso é rápido O(1) e lida com colisões.
     */
    newSym->next = symbolTable[idx];
    symbolTable[idx] = newSym;
    
    /* Logs de Debug para acompanhar a compilação */
    if (kind == KIND_FUNCTION) {
         printf("[DEBUG] Funcao '%s' declarada (Global).\n", name);
    } else if (kind == KIND_SCALAR) {
         printf("[DEBUG] Var '%s' instalada no Escopo %d.\n", name, current_scope);
    } else if (kind == KIND_UNIT) { 
         printf("[DEBUG] Unit/Struct '%s' instalada.\n", name);
    } else if(kind == KIND_ARRAY) {
        printf("[DEBUG] Array '%s'[%d] instalado.\n", name, size1);
    } else {
        printf("[DEBUG] Matriz '%s'[%d][%d] instalada.\n", name, size1, size2);
    }
}

/* Função utilitária para visualizar o estado atual da tabela */
void print_symbol_table() {
    printf("\n--- Tabela de Simbolos ---\n");
    for (int i = 0; i < TABLE_SIZE; i++) {
        Symbol *sym = symbolTable[i];
        if (sym != NULL) {
            printf("[%d]: ", i);
            while (sym != NULL) {
                printf("%s (Escopo %d) -> ", sym->name, sym->scope);
                sym = sym->next;
            }
            printf("NULL\n");
        }
    }
    printf("--------------------------\n");
}