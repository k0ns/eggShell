CC = gcc

all: eggShell.c 
	gcc eggShell.c -o eggShell

clean: 
	$(RM) eggShell
