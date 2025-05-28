ASM := $(wildcard *.asm)
BIN := $(ASM:.asm=) 

all: $(BIN)

%: %.asm
	nasm -f elf64 $< -o $@.o
	ld $@.o -o $@

clean:
	rm -f *.o $(BIN)

