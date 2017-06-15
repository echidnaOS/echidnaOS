kernel/echidna.bin: target_libc ./shell/shell.bin
	cd kernel && make

target_libc:
	cd libc && make

./shell/shell.bin:
	cd shell && make

.PHONY: clean clean-all img all

clean:
	cd shell && make clean
	cd libc && make clean
	cd kernel && make clean

clean-all:
	rm ./shell/shell.bin
	rm ./kernel/echidna.bin

img:
	nasm bootloader/bootloader.asm -f bin -o echidna.img
	dd bs=512 count=131032 if=/dev/zero >> ./echidna.img
	mkdir mnt
	mount ./echidna.img ./mnt
	cp ./kernel/echidna.bin ./mnt/echidna.bin
	sync
	umount ./mnt
	chown `logname`:`logname` ./echidna.img
	rm -rf ./mnt

all:
	make
	make clean
	make img
	make clean-all
