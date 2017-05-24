CC = ../gcc/bin/i686-elf-gcc
LD = ../gcc/bin/i686-elf-ld

C_FILES = $(shell find ./ -type f -name '*.c')
LIBC_FILES = $(shell find ../libc/ -type f -name '*.c')
C_OBJ = $(C_FILES:.c=.o) $(LIBC_FILES:.c=.o)

CFLAGS = -std=gnu99 -isystem ../libc/ -masm=intel

shell.bin: $(C_OBJ)
	$(LD) -T linker.ld --oformat binary -o shell.bin -nostdlib $(C_OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm $(C_OBJ)
