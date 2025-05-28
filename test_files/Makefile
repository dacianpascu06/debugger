CC = gcc
CFLAGS = -Wall -Wextra -O2 

SRCS := $(wildcard *.c)
EXES := $(SRCS:.c=)

all: $(EXES)

%: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(EXES)

