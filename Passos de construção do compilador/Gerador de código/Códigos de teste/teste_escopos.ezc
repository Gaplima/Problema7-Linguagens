/* --- 1. Declarações Globais --- */
int global;

/* --- 2. Funções Auxiliares (Único lugar onde var local é permitida) --- */

/* Teste (D) Shadowing */
int teste_shadowing(int a) begin
    int global;   /* Variável LOCAL com mesmo nome da global */
    global := 20;
    echo(global); /* Deve imprimir 20 (Local) */
end

/* Teste (C) Escopos Distintos - Parte 1 */
int escopo_um() begin
    int x;
    x := 5;
    echo(x);
end

/* Teste (C) Escopos Distintos - Parte 2 */
int escopo_dois() begin
    int x;        /* Novo 'x', sem conflito com o de cima */
    x := 99;
    echo(x);
end

/* --- 3. Execução Principal (O compilador coloca isso no main) --- */

global := 10; /* Inicializa a global */

echo("--- Inicio ---");
echo(global);       /* Imprime 10 (Global) */

echo("--- Teste Shadowing ---");
teste_shadowing(0); /* Chama a função, imprime 20 */
echo(global);       /* Imprime 10 novamente (Global intocada) */

echo("--- Teste Escopos Distintos ---");
/* Simulamos blocos isolados chamando funções */
if (1 < 2) then begin
    escopo_um();    /* Imprime 5 */
end

if (1 < 2) then begin
    escopo_dois();  /* Imprime 99 */
end

/* Teste (A) Variável omitida - Descomente a linha abaixo para ver o ERRO */
/* y := 50; */

/* Teste (B) Duplicada no mesmo escopo - Descomente para ver o ERRO */
/* int z; int z; */ /* (Isso daria erro de sintaxe aqui no main, teste no topo) */