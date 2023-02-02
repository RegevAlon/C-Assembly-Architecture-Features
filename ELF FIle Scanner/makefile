all: link

link: compile
	gcc -m32 -g -Wall -o myELF myELF.c
	
compile: myELF.c
	gcc -m32 -g -Wall -c -o myELF.o myELF.c
	
clean:
	rm -f *.o myELF
