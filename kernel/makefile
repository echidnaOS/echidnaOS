export PATH := ../tools/bin:$(PATH)
CC = kcc
XXD = xxd
PREFIX = 

C_FILES = init.c $(shell find src -type f -name '*.c')
H_FILES = $(shell find . -type f -name '*.h')
ASM_FILES = $(shell find asm -type f -name '*.asm')
REAL_FILES = $(shell find blobs -type f -name '*.real')
OBJ = $(C_FILES:.c=.o) $(ASM_FILES:.asm=.o)
BINS = $(REAL_FILES:.real=.bin)
INITRAMFS = initramfs

CHARDFLAGS = -std=gnu99 -masm=intel
CFLAGS = -O2

echidna.bin: initramfs_obj.o $(BINS) $(OBJ) $(H_FILES)
	$(CC) -o echidna.bin $(OBJ) initramfs_obj.o

initramfs_obj.o: $(INITRAMFS)
	$(XXD) -i $(INITRAMFS) > initramfs.c
	$(CC) $(CHARDFLAGS) $(CFLAGS) -c initramfs.c -o initramfs_obj.o

%.o: %.c
	$(CC) $(CHARDFLAGS) $(CFLAGS) -c $< -o $@

%.bin: %.real
	nasm $< -f bin -o $@

%.o: %.asm
	nasm $< -f elf32 -o $@

clean:
	rm -f $(OBJ) $(BINS) echidna.bin
	rm -f initramfs.c initramfs_obj.o

install:
	cp echidna.bin $(PREFIX)/
