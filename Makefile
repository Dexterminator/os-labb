all: build

build: shell.c
	gcc -pedantic -Wall -ansi -O4 -o shell shell.c cd.c checkenv.c helper.c
