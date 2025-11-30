/* --- TODAS AS DECLARAÇÕES DEVEM FICAR NO TOPO --- */
/* Variaveis do Problema 1 */
float x;
float y;
int c;
float resultado;

/* Variaveis do Problema 2 */
int num;

/* --- INICIO DOS COMANDOS --- */

/* Lógica Problema 1 */
x := 5.5;
y := 2.0;
c := 10;

resultado := x^2 - y + c;

echo("Resultado da Expressao:");
echo(resultado);

/* Lógica Problema 2 */
num := 30;

echo("Verificando numero 30...");

if num >= 26 and num <= 50 then
    echo("O numero esta entre 26 e 50");
else
    echo("O numero esta fora do intervalo");