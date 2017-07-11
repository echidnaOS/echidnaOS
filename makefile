kernel/echidna.bin: target_libc ./shell/shell.bin
	cd kernel && make

target_libc:
	cd libc && make

./shell/shell.bin:
	cd shell && make

echidnafs/echfs-utils: echidnafs/echfs-utils.c
	cd echidnafs && gcc echfs-utils.c -o echfs-utils

.PHONY: clean clean-all img all compiler

clean:
	cd shell && make clean
	cd libc && make clean
	cd kernel && make clean

clean-all:
	rm ./shell/shell.bin
	rm ./kernel/echidna.bin

img: echidnafs/echfs-utils
	nasm bootloader/bootloader.asm -f bin -o echidna.img
	dd bs=512 count=131032 if=/dev/zero >> ./echidna.img
	echidnafs/echfs-utils echidna.img format
	echidnafs/echfs-utils echidna.img import ./kernel/echidna.bin echidna.bin

all:
	make
	make clean
	make img
	make clean-all

compiler: gcc-6.3.0.tar.bz2 binutils-2.28.tar.bz2
	sha256sum -c sha256packages
	tar -vxf gcc-6.3.0.tar.bz2
	tar -vxf binutils-2.28.tar.bz2
	export MAKEFLAGS="-j `grep -c ^processor /proc/cpuinfo`" && export PREFIX="`pwd`/gcc" && export TARGET=i386-elf && export PATH="$$PREFIX/bin:$$PATH" && mkdir build-binutils && cd build-binutils && ../binutils-2.28/configure --target=$$TARGET --prefix="$$PREFIX" --with-sysroot --disable-nls --disable-werror && make && make install && cd ../gcc-6.3.0 && contrib/download_prerequisites && cd .. && mkdir build-gcc && cd build-gcc && ../gcc-6.3.0/configure --target=$$TARGET --prefix="$$PREFIX" --disable-nls --enable-languages=c --without-headers && make all-gcc && make all-target-libgcc && make install-gcc && make install-target-libgcc
	rm -rf gcc-6.3.0 binutils-2.28 build-gcc build-binutils

gcc-6.3.0.tar.bz2:
	wget ftp://ftp.gnu.org/gnu/gcc/gcc-6.3.0/gcc-6.3.0.tar.bz2

binutils-2.28.tar.bz2:
	wget ftp://ftp.gnu.org/gnu/binutils/binutils-2.28.tar.bz2
