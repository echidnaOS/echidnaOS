CC = ../gcc/bin/i386-elf-gcc

C = $(shell find ./ -type f,l -name '*.c')
OBJ = $(C:.c=.o)

CFLAGS = -std=gnu99 -ffreestanding -isystem ./ -masm=intel

libc: $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm $(OBJ)
