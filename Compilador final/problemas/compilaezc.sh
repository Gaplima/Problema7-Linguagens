rm *.c # remove todos os .c
for f in *.ezc; do ../compilador "$f"; done #compila todos os .ezc