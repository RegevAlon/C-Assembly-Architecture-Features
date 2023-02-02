all: link

link: compile
	gcc -m32 -g -Wall -o virus_detector virus_detector.c
	
compile: virus_detector.c
	gcc -m32 -g -Wall -c -o virus_detector.o virus_detector.c
	
clean:
	rm -f *.o virus_detector
