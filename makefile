ARCH = i686

CC = ./gcc/bin/i686-elf-gcc

C_FILES = $(shell find kernel/ -type f -name '*.c')
ASM_FILES = $(shell find kernel/asm/$(ARCH) -type f -name '*.asm')
REAL_FILES = $(shell find kernel/asm/$(ARCH) -type f -name '*.real')
LIBC_FILES = $(shell find libc/ -type f -name '*.c')
C_OBJ = $(C_FILES:.c=.o) $(LIBC_FILES:.c=.o)
ASM_OBJ = $(ASM_FILES:.asm=.o)
OBJ = boot/$(ARCH)/boot.o $(C_OBJ) $(ASM_OBJ)
BINS = $(REAL_FILES:.real=.bin)

CFLAGS = -std=gnu99 -O2 -Wall -Wextra -isystem libc/ -isystem kernel/global/ -masm=intel -D__ARCH_$(ARCH)__

ifeq ($(ARCH),i686)
	NASMFLAGS = -f elf32
endif
ifeq ($(ARCH),amd64)
	NASMFLAGS = -f elf64
endif

echidna.bin: $(BINS) $(OBJ)
	$(CC) -T linker.ld -o echidna.bin -O2 -nostdlib $(OBJ) -lgcc

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.bin: %.real
	nasm $< -f bin -o $@

%.o: %.asm
	nasm $< $(NASMFLAGS) -o $@

.PHONY: clean img

clean:
	rm $(OBJ) $(BINS)

img:
	dd bs=512 count=131040 if=/dev/zero of=./echidna.img
	losetup /dev/loop0 ./echidna.img
	mkfs.vfat -F 16 -n ECHIDNA -I /dev/loop0
	mount /dev/loop0 /mnt
	mkdir -p /mnt/boot/grub
	cp echidna.bin /mnt/boot/echidna.bin
	cp boot/grub.cfg /mnt/boot/grub/grub.cfg
	sync
	grub-install --root-directory=/mnt --no-floppy --recheck --force /dev/loop0
	sync
	umount /dev/loop0
	losetup -d /dev/loop0
	chown `logname`:`logname` ./echidna.img
