rm *.c # remove todos os .c
for f in *.sc; do ../compilador "$f"; done #compila todos os .sc