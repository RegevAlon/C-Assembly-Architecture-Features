all: link

link: compile
	gcc -m32 -g -Wall -o hexeditplus hexeditplus.c
	
compile: hexeditplus.c
	gcc -m32 -g -Wall -c -o hexeditplus.o hexeditplus.c
	
clean:
	rm -f *.o hexeditplus
