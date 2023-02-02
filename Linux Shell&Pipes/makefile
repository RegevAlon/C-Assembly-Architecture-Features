all: myshell

myshell: myShell.o LineParser.o
	gcc -g -m32 -Wall -o myshell myShell.o LineParser.o
myShell.o: myShell.c
	gcc -m32 -g -Wall -c -o myShell.o myShell.c
LineParser.o: LineParser.c
	gcc -m32 -g -Wall -c -o LineParser.o LineParser.c
clean:
	rm -f *.o myshell
