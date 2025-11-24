# Projeto de Engenharia de Linguagens
Projeto para a disciplina de Engenharia de Linguagens, 2025.2

### Como compilar?

- Clone a versão mais recente do compilador e use os comandos:

```
bison -dy parser.y
flex lexer.l
gcc lex.yy.c y.tab.c symbol_table.c ast.c -o compilador
```
- Rode o compilador com:
``` ./compilador < arquivo_da_linguagem ```
