flex cminus.l
bison -d cminus.y
gcc -c *.c
gcc -o cminus *.o -ly -lfl

flex cminus.l && bison -d cminus.y && gcc -c *.c && gcc -o cminus *.o -ly -lfl

./cminus -l -y -a -c -f 1.cm

-l -> analisador léxico
-y -> analisador sintático
-a -> analisador semântico
-c -> comentários assembly
-f -> nome do arquivo (obrigatório)