flex tiny.l 
bison -d tiny.y
gcc -c *.c
gcc -o tiny *.o -ly -lfl

flex tiny.l && bison -d tiny.y && gcc -c *.c && gcc -o tiny *.o -ly -lfl


./tiny -l -y -a -f 2.tny

-l -> analisador léxico
-y -> analisador sintático
-a -> analisador semântico
-f -> nome do arquivo (obrigatório)