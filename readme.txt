flex cminus.l
bison -d cminus.y
gcc -c *.c
gcc -o cminus *.o -ly -lfl

flex cminus.l && bison -d cminus.y && gcc -c *.c && gcc -o cminus *.o -ly -lfl

./cminus -l -y -a -f 2.tny

-l -> analisador léxico
-y -> analisador sintático
-a -> analisador semântico
-f -> nome do arquivo (obrigatório)