	
link: compile
	ld -m elf_i386 start.o main.o util.o -o detector
compile: util.c 
	nasm -f elf32 start.s -o start.o 
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector main.c -o main.o
clean:
	@echo "Cleaning bin directory"
	rm -f *.o detector main
