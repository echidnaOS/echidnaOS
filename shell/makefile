CC = ../gcc/bin/i686-elf-gcc
LD = ../gcc/bin/i686-elf-ld

C = $(shell find ./ -type f -name '*.c')
OBJ = $(C:.c=.o)
LIBC = $(shell find ../libc/ -type f -name '*.o')

CFLAGS = -std=gnu99 -ffreestanding -isystem ../libc/ -masm=intel

shell.bin: $(OBJ) $(LIBC)
	$(LD) -T linker.ld --oformat binary -o shell.bin -nostdlib $(OBJ) $(LIBC)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm $(OBJ)
