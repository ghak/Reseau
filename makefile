all:test.c udpc1.c
	gcc -o client udpc1.c
	gcc -o test test.c
