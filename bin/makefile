assembler: main.o utils.o macro.o first_pass.o structLabels.o struct_ext.o second_pass.o 
	gcc -g -Wall -ansi -pedantic main.o  macro.o  first_pass.o struct_ext.o second_pass.o utils.o structLabels.o -lm -o assembler

main.o: main.c main.h prototypes.h assembler.h ext_vars.h structs.h utils.h
	gcc -c main.c -o main.o 

utils.o: utils.c utils.h prototypes.h assembler.h ext_vars.h structs.h
	gcc -c utils.c -o utils.o 

macro.o: macro.c macro.h
	gcc -c macro.c -o macro.o
	
first_pass.o: first_pass.c prototypes.h assembler.h ext_vars.h structs.h utils.h
	gcc -c first_pass.c -o first_pass.o
	
structLabels.o: structLabels.c prototypes.h assembler.h ext_vars.h structs.h utils.h
	gcc -c structLabels.c -o structLabels.o
	
second_pass.o: second_pass.c prototypes.h assembler.h ext_vars.h structs.h
	gcc -c second_pass.c -o second_pass.o

struct_ext.o: struct_ext.c prototypes.h assembler.h ext_vars.h structs.h
	gcc -c struct_ext.c -o struct_ext.o
