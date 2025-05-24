ASM       := nasm
ASFLAGS   := -f elf64

LD        := ld
LDFLAGS   := -m elf_x86_64 -static -nostdlib --no-pie

# collect all .asm files and derive .o and target names
ASM_SRCS  := $(wildcard *.asm)
OBJS      := $(ASM_SRCS:.asm=.o)
TARGETS   := $(ASM_SRCS:.asm=)

.PHONY: all clean

all: $(TARGETS)

# Link rule: every target (e.g. foo) comes from foo.o
# $@ = target, $< = first prerequisite (the .o)
$(TARGETS): %: %.o
	$(LD) $(LDFLAGS) --entry=main -o $@ $<

# Assemble rule: every .o from its .asm
# $@ = the .o, $< = the .asm
%.o: %.asm
	$(ASM) $(ASFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS) $(OBJS)

