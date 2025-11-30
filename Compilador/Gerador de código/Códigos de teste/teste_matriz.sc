int mat := [3][3]; /* CORREÇÃO: Usar := ao invés de = */
int i;
int j;

for i := 0 to 2 do begin
    for j := 0 to 2 do begin
        if i == j then
            mat[i][j] := 1;
        else
            mat[i][j] := 0;
    end
end

/* Imprime a diagonal principal */
echo(mat[0][0]);
echo(mat[1][1]);
echo(mat[2][2]);