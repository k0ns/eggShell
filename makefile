CC = gcc

all: eggShell.c 
	gcc -Wall eggShell.c -o eggShell

clean: 
	$(RM) eggShell
