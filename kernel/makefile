CC = ../gcc/bin/i386-elf-gcc
LD = ../gcc/bin/i386-elf-ld

C_FILES = ./init.c $(shell find ./common/ -type f,l -name '*.c')
H_FILES = $(shell find ./ -type f,l -name '*.h')
ASM_FILES = $(shell find ./asm/ -type f,l -name '*.asm')
REAL_FILES = $(shell find ./blobs/ -type f,l -name '*.real')
OBJ = $(C_FILES:.c=.o) $(ASM_FILES:.asm=.o)
BINS = $(REAL_FILES:.real=.bin)

CFLAGS = -std=gnu99 -ffreestanding -isystem ./include -masm=intel

echidna.bin: ../shell/shell.bin $(BINS) $(OBJ) $(H_FILES)
	$(LD) -T linker.ld --oformat binary -o echidna.bin -nostdlib $(OBJ)

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
