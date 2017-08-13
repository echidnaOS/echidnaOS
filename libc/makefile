CC = ../tools/bin/i386-elf-gcc

C_FILES = $(shell find ./ -type f -name '*.c')
OBJ_FILES = $(C_FILES:.c=.o)

CFLAGS = -std=gnu99 -ffreestanding -isystem ./ -masm=intel -mpreferred-stack-boundary=2

libc: $(OBJ_FILES)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm $(OBJ_FILES)
