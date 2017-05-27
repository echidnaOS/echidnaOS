CC = ./gcc/bin/i686-elf-gcc

C_FILES = $(shell find kernel/ -type f -name '*.c')
ASM_FILES = $(shell find kernel/ -type f -name '*.asm')
REAL_FILES = $(shell find kernel/ -type f -name '*.real')
C_OBJ = $(C_FILES:.c=.o)
ASM_OBJ = $(ASM_FILES:.asm=.o)
OBJ = boot/boot.o $(C_OBJ) $(ASM_OBJ)
BINS = $(REAL_FILES:.real=.bin)

CFLAGS = -std=gnu99 -ffreestanding -isystem kernel/global/ -masm=intel
NASMFLAGS = -f elf32

echidna.bin:  ./shell/shell.bin $(BINS) $(OBJ)
	$(CC) -T linker.ld -o echidna.bin -nostdlib $(OBJ) -lgcc

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

iso:
	mkdir -p isodir/boot/grub
	cp echidna.bin isodir/boot/echidna.bin
	cp boot/grub.cfg isodir/boot/grub/grub.cfg
	grub-mkrescue -o echidna.iso isodir
	rm -rf isodir
	chown `logname`:`logname` ./echidna.iso
