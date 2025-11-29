/* Declaração Global */
int res;

int fatorial(int n) begin
    if n <= 1 then
        return 1;
    else
        return n * fatorial(n - 1);
end

/* Programa Principal */
res := fatorial(5);
echo(res); /* Deve imprimir 120 */