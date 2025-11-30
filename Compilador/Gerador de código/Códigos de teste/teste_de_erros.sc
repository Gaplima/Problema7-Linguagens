/* ========================================================== */
/* ARQUIVO DE TESTE DE ERROS - COMPILADOR ESTRITO             */
/* Descomente UM caso por vez para ver a mensagem de erro.    */
/* ========================================================== */

/* --- 1. SEÇÃO DE DECLARAÇÕES GLOBAIS --- */
int g;

/* [ERRO B] - TESTE DE DUPLICIDADE (Global) */
/* Esperado: Mensagem "ERRO: g duplicado" (Semantico) */
/* int g; */ 

/* [ERRO A] - TESTE DE DECLARAÇÃO ESTILO C (Errado nesta linguagem) */
/* A linguagem exige := para arrays. int arr[10]; não existe. */
/* Esperado: Erro de Sintaxe (Parser não reconhece '[' logo após ID na declaração) */
/* int lista[10]; */

/* --- 2. DEFINIÇÃO DE FUNÇÕES --- */
int funcaoTeste(int p) begin
    /* Declarações locais devem vir AGORA, logo após o begin */
    int local;

    local := 10;
    
    /* [ERRO A] - DECLARAÇÃO APÓS COMANDO */
    /* Como já houve um comando (local := 10), não pode mais declarar. */
    /* Esperado: Erro de Sintaxe (stmt_list não aceita declarations) */
    /* int atrasada; */

    /* [ERRO B] - DUPLICIDADE DE PARÂMETRO E LOCAL */
    /* Se descomentar, deve acusar colisão com o parametro 'p' ou redeclaração */
    /* int p; */
end

/* --- 3.BLOCO PRINCIPAL (MAIN) --- */

g := 10;

/* [ERRO A] - DECLARAÇÃO DENTRO DE BLOCO IF */
/* Declarações não são permitidas dentro de comandos (if, while, etc) */
/* Esperado: Erro de Sintaxe (stmt não tem transição para type/declaration) */
if (g > 5) then begin
   /* int x; */ 
   echo(g);
end

/* [ERRO B] - VARIÁVEL NÃO DECLARADA */
/* Esperado: "ERRO: 'fantasma' nao declarado." (Semantico) */
/* fantasma := 999; */

/* [ERRO A] - ATRIBUIÇÃO ERRADA (Estilo C) */
/* A linguagem usa :=, e não = */
/* Esperado: Erro de Sintaxe */
/* g = 20; */

/* [ERRO C] - USO INDEVIDO DE TIPO */
/* Tentando usar escalar como array */
/* Esperado: "ERRO: 'g' nao array." (Semantico) */
/* g[0] := 1; */

/* [ERRO B] - CHAMADA DE FUNÇÃO INEXISTENTE */
/* Esperado: "ERRO: 'naoExiste' nao e funcao." (Semantico) */
/* naoExiste(); */

echo(g);