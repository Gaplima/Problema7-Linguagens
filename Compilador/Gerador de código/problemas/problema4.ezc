/* Problema 4: Tipo Racional (Struct) */

/* Definicao do tipo rational_t */
unit rational_t begin
    int numerador;
    int denominador;
end

/* Funcao que cria e retorna um rational_t */
unit rational_t criar_racional(int a, int b) begin
    unit rational_t res;
    
    res.numerador := a;
    res.denominador := b;
    
    return res;
end

/* Programa Principal */
unit rational_t r1;
int x;
int y;

echo("--- Racionais ---");
echo("Numerador:");
read(x);

echo("Denominador:");
read(y);

if y == 0 then
begin
    echo("Erro: Denominador zero.");
end
else
begin
    r1 := criar_racional(x, y);
    
    echo("Fracao resultante:");
    echo(r1.numerador);
    echo("/");
    echo(r1.denominador);
end