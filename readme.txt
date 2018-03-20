flex tiny.l 
bison -d tiny.y
gcc -g -c *.c
gcc -o tiny *.o -ly -lfl

flex tiny.l && bison -d tiny.y && gcc -g -c *.c && gcc -o tiny *.o -ly -lfl
gdb ./tiny 1

