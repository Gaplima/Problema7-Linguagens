/* Problema 2: Contar numeros em intervalos especificos */

/* Variavel de entrada e contadores */
int num;
int c1; /* Intervalo 0-25 */
int c2; /* Intervalo 26-50 */
int c3; /* Intervalo 51-75 */
int c4; /* Intervalo 76-100 */

/* Inicializacao */
c1 := 0;
c2 := 0;
c3 := 0;
c4 := 0;
num := 0;

echo("Digite numeros (digite negativo para sair):");

/* Loop de leitura indeterminada */
while num >= 0 do
begin
    read(num);

    /* Apenas processa se for nao-negativo */
    if num >= 0 then
    begin
        if num <= 25 then
            c1 := c1 + 1;
        
        if num >= 26 and num <= 50 then
            c2 := c2 + 1;
            
        if num >= 51 and num <= 75 then
            c3 := c3 + 1;
            
        if num >= 76 and num <= 100 then
            c4 := c4 + 1;
    end
end

/* Exibicao dos resultados */
echo("--- Contagem por Intervalo ---");
echo("[0, 25]:");
echo(c1);
echo("[26, 50]:");
echo(c2);
echo("[51, 75]:");
echo(c3);
echo("[76, 100]:");
echo(c4);