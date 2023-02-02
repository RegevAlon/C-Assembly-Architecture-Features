all: link

link: compile
	gcc -m32 -g -Wall -o start start.o
	
compile: start.s
	nasm -g -f elf -w+all -o start.o start.s
	
clean:
	rm -f *.o start
