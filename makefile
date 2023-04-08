# makefile

OBJ = main.o

all: main

main: $(OBJ)
	gcc -o main $(OBJ) -lm -lc

main.o: main.c main.h
	gcc -c main.c

doxygen:
	doxygen Doxyfile
	
clean:
	rm -f main $(OBJ)