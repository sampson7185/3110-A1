CC = gcc
CFLAGS = -Wall -pedantic -std=c99 -g -c

all: myShell

myShell: myShell.o
	$(CC) myShell.o -o CIS3110sh

myShell.o: myShell.c
	$(CC) $(CFLAGS) myShell.c

clean:
	rm *o myShell