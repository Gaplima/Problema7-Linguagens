/* Problema 3: Soma e Produto de Matrizes */

/* Declaracao de matrizes e variaveis */
float matA := [5][5];
float matB := [5][5];
float matSoma := [5][5];
float matProd := [5][5];
int linA;
int colA;
int linB;
int colB;
int i;
int j;
int k;
float acc;

/* Leitura das dimensoes da Matriz A */
echo("--- MATRIZ A ---");
echo("Linhas de A:");
read(linA);
echo("Colunas de A:");
read(colA);

/* Ajuste para indice 0 */
linA := linA - 1;
colA := colA - 1;

/* Leitura dos elementos da Matriz A */
echo("Digite elementos de A:");
for i := 0 to linA do
begin
    for j := 0 to colA do
    begin
        read(matA[i][j]);
    end
end

/* Leitura das dimensoes da Matriz B */
echo("--- MATRIZ B ---");
echo("Linhas de B:");
read(linB);
echo("Colunas de B:");
read(colB);

/* Ajuste para indice 0 */
linB := linB - 1;
colB := colB - 1;

/* Leitura dos elementos da Matriz B */
echo("Digite elementos de B:");
for i := 0 to linB do
begin
    for j := 0 to colB do
    begin
        read(matB[i][j]);
    end
end

echo(" ");
echo("--- SOMA ---");

/* Verifica se as dimensoes sao compativeis para soma */
if linA == linB and colA == colB then
begin
    echo("Resultado Soma:");
    for i := 0 to linA do
    begin
        for j := 0 to colA do
        begin
            matSoma[i][j] := matA[i][j] + matB[i][j];
            echo(matSoma[i][j]);
        end
    end
end
else
begin
    echo("Dimensoes invalidas para Soma");
end

echo(" ");
echo("--- PRODUTO ---");

/* Verifica se as dimensoes sao compativeis para multiplicacao */
if colA == linB then
begin
    echo("Resultado Produto:");
    for i := 0 to linA do
    begin
        for j := 0 to colB do
        begin
            acc := 0.0;
            
            /* Calculo do produto escalar */
            for k := 0 to colA do
            begin
                acc := acc + (matA[i][k] * matB[k][j]);
            end
            
            matProd[i][j] := acc;
            echo(matProd[i][j]);
        end
    end
end
else
begin
    echo("Dimensoes invalidas para Produto");
end