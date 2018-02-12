CC = x86_64-echidnaos-gcc
XXD = xxd

C_FILES = $(shell find src -type f -name '*.c')
H_FILES = $(shell find . -type f -name '*.h')
ASM_FILES = $(shell find asm -type f -name '*.asm') $(shell find startup -type f -name '*.asm')
REAL_FILES = $(shell find blobs -type f -name '*.real')
OBJ = $(C_FILES:.c=.o) $(ASM_FILES:.asm=.o)
BINS = $(REAL_FILES:.real=.bin)
INITRAMFS = initramfs

CHARDFLAGS = -std=gnu99 -masm=intel \
    -mno-sse \
    -nostdlib \
    -ffreestanding \
    -I./include/

CLINKFLAGS = -nostdlib \
    -static-libgcc -lgcc \
    -T ./linker.ld

CFLAGS = -O2

echidna.bin: initramfs_obj.o $(BINS) $(OBJ) $(H_FILES)
	$(CC) $(OBJ) initramfs_obj.o $(CLINKFLAGS) -o echidna.bin

initramfs_obj.o: $(INITRAMFS)
	$(XXD) -i $(INITRAMFS) > initramfs.c
	$(CC) $(CHARDFLAGS) $(CFLAGS) -c initramfs.c -o initramfs_obj.o

%.o: %.c
	$(CC) $(CHARDFLAGS) $(CFLAGS) -c $< -o $@

%.bin: %.real
	nasm $< -f bin -o $@

%.o: %.asm
	nasm $< -f elf64 -o $@

clean:
	rm -f $(OBJ) $(BINS) echidna.bin
	rm -f initramfs.c initramfs_obj.o
