rm lex.yy.c y.tab.c y.tab.h
bison -dy parser.y
flex lexer.l
gcc lex.yy.c y.tab.c symbol_table.c ast.c codegen.c -o compilador