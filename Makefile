CC = gcc
CFLAGS = -Wall -g
OBJ = main.o elf_parser/elf_parser.o utils/utils.o

all: debugger

debugger: $(OBJ)
	$(CC) -o debugger $(OBJ)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

elf_parser/elf_parser.o: elf_parser/elf_parser.c elf_parser/elf_parser.h utils/utils.h
	$(CC) $(CFLAGS) -c elf_parser/elf_parser.c -o elf_parser/elf_parser.o

utils/utils.o: utils/utils.c utils/utils.h
	$(CC) $(CFLAGS) -c utils/utils.c -o utils/utils.o

clean:
	rm -f $(OBJ) debugger

