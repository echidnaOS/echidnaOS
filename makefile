CC = cc
LD = ld

C_FILES = $(wildcard kernel/*.c wildcard libc/*.c wildcard kernel/libs/*.c wildcard kernel/drivers/*.c wildcard kernel/shell/*.c)
ASM_FILES = $(wildcard kernel/drivers/*.asm)
C_OBJ = ${C_FILES:.c=.o}
ASM_OBJ = ${ASM_FILES:.asm=.o}
OBJ = ${C_OBJ} ${ASM_OBJ}

CFLAGS = -std=gnu99 -nostdlib -nostartfiles -nodefaultlibs -fno-builtin -isystem libc/ -isystem kernel/global/ -masm=intel -m32
LDFLAGS = -T linker_script --oformat binary -melf_i386
NASMFLAGS = -f elf32

echidna.img: kernel.sys bootloader.bin
	cp bootloader.bin echidna.img
	dd bs=512 count=2872 status=none if=/dev/zero >> echidna.img
	mkdir mnt
	mount echidna.img ./mnt
	cp kernel.sys mnt/
	sync
	umount ./mnt
	rm -rf mnt
	rm -rf ${OBJ} bootloader.bin kernel.sys

bootloader.bin: bootloader/bootloader.asm
	nasm bootloader/bootloader.asm -f bin -o bootloader.bin

kernel.sys: ${OBJ}
	${LD} ${LDFLAGS} $^ -o $@

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

%.o: %.asm
	nasm $< ${NASMFLAGS} -o $@

clean:
	rm -rf mnt
	rm -rf ${OBJ} bootloader.bin kernel.sys
