all: build

build: shell.c
	gcc -pedantic -Wall $(CFLAGS) -ansi -O4 -o shell shell.c cd.c checkenv.c helper.c exit.c
