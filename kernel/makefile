CC = ../tools/bin/i386-elf-gcc

C_FILES = ./init.c $(shell find ./common/ -type f -name '*.c')
H_FILES = $(shell find ./ -type f -name '*.h')
ASM_FILES = $(shell find ./asm/ -type f -name '*.asm')
REAL_FILES = $(shell find ./blobs/ -type f -name '*.real')
OBJ = $(C_FILES:.c=.o) $(ASM_FILES:.asm=.o)
BINS = $(REAL_FILES:.real=.bin)

CFLAGS = -std=gnu99 -ffreestanding -isystem ./include -masm=intel

echidna.bin: ../shell/shell.bin $(BINS) $(OBJ) $(H_FILES)
	$(CC) -T linker.ld -o echidna.bin -nostdlib $(OBJ) -static-libgcc -lgcc -Xlinker --oformat=binary

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.bin: %.real
	nasm $< -f bin -o $@

%.o: %.asm
	nasm $< -f elf32 -o $@

../shell/shell.bin:
	$(error Unable to find shell.bin)

.PHONY: clean

clean:
	rm $(OBJ) $(BINS)
