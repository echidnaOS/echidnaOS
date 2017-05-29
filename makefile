CC = ./gcc/bin/i686-elf-gcc
LD = ./gcc/bin/i686-elf-ld

C_FILES = $(shell find kernel/ -type f -name '*.c')
ASM_FILES = $(shell find kernel/ -type f -name '*.asm')
REAL_FILES = $(shell find kernel/ -type f -name '*.real')
C_OBJ = $(C_FILES:.c=.o)
ASM_OBJ = $(ASM_FILES:.asm=.o)
OBJ = $(C_OBJ) $(ASM_OBJ)
BINS = $(REAL_FILES:.real=.bin)

CFLAGS = -std=gnu99 -ffreestanding -isystem kernel/global/ -masm=intel
NASMFLAGS = -f elf32

echidna.bin:  ./shell/shell.bin $(BINS) $(OBJ)
	$(LD) -T linker.ld --oformat binary -o echidna.bin -nostdlib $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.bin: %.real
	nasm $< -f bin -o $@

%.o: %.asm
	nasm $< $(NASMFLAGS) -o $@

./shell/shell.bin:
	cd shell && make
	cd shell && make clean

.PHONY: clean img

clean:
	rm shell/shell.bin
	rm $(OBJ) $(BINS)

img:
	nasm bootloader/bootloader.asm -f bin -o echidna.img
	dd bs=512 count=131032 if=/dev/zero >> ./echidna.img
	mkdir mnt
	mount ./echidna.img ./mnt
	cp echidna.bin ./mnt/echidna.bin
	sync
	umount ./mnt
	chown `logname`:`logname` ./echidna.img
	rm -rf ./mnt
