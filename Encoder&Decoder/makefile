all: link

link: compile
	gcc -m32 -g -Wall -o menu_map menu_map.c
	
compile: menu_map.c
	gcc -m32 -g -Wall -c -o menu_map.o menu_map.c
	
clean:
	rm -f *.o menu_map
