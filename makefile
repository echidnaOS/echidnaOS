CC = ./gcc/bin/i686-elf-gcc

C_FILES = $(shell find kernel/ -type f -name '*.c')
ASM_FILES = $(shell find kernel/ -type f -name '*.asm')
LIBC_FILES = $(shell find libc/ -type f -name '*.c')
C_OBJ = $(C_FILES:.c=.o) $(LIBC_FILES:.c=.o)
ASM_OBJ = $(ASM_FILES:.asm=.o)
OBJ = boot/boot.o $(C_OBJ) $(ASM_OBJ)

CFLAGS = -std=gnu99 -O2 -Wall -Wextra -isystem libc/ -isystem kernel/global/ -masm=intel
NASMFLAGS = -f elf32

echidna.bin: $(OBJ)
	$(CC) -T linker.ld -o echidna.bin -O2 -nostdlib $(OBJ) -lgcc

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	nasm $< $(NASMFLAGS) -o $@

.PHONY: clean iso iso-clean test-qemu-bin test-qemu-iso

clean:
	rm $(OBJ)

iso:
	mkdir -p isodir/boot/grub
	cp echidna.bin isodir/boot/echidna.bin
	cp boot/grub.cfg isodir/boot/grub/grub.cfg
	grub-mkrescue -o echidna.iso isodir

iso-clean:
	rm -rf isodir/

test-qemu-bin:
	qemu-system-i386 -kernel echidna.bin

test-qemu-iso:
	qemu-system-i386 -cdrom echidna.iso
