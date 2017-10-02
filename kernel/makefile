export PATH := ../tools/bin:$(PATH)
CC = kcc
PREFIX = 

C_FILES = init.c $(shell find src -type f -name '*.c')
H_FILES = $(shell find . -type f -name '*.h')
ASM_FILES = $(shell find asm -type f -name '*.asm')
REAL_FILES = $(shell find blobs -type f -name '*.real')
OBJ = $(C_FILES:.c=.o) $(ASM_FILES:.asm=.o)
BINS = $(REAL_FILES:.real=.bin)

CHARDFLAGS = -std=gnu99 -masm=intel
CFLAGS = -O1

echidna.bin: $(BINS) $(OBJ) $(H_FILES)
	$(CC) -o echidna.bin $(OBJ)

%.o: %.c
	$(CC) $(CHARDFLAGS) $(CFLAGS) -c $< -o $@

%.bin: %.real
	nasm $< -f bin -o $@

%.o: %.asm
	nasm $< -f elf32 -o $@

clean:
	rm -f $(OBJ) $(BINS) echidna.bin

install:
	cp echidna.bin $(PREFIX)/
