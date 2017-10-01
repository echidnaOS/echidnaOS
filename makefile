LIBC_C_FILES = $(shell find libc -type f -name '*.c')
COREINUTILS_C_FILES = $(shell find coreinutils/src -type f -name '*.c')
KERNEL_ASM_FILES = $(shell find kernel -type f -name '*.asm')
KERNEL_C_FILES = $(shell find kernel -type f -name '*.c')

notarget:
	$(error No target specified)

libc/libc: $(LIBC_C_FILES)
	$(MAKE) -C libc

coreinutils/coreinutils: libc/libc $(COREINUTILS_C_FILES)
	$(MAKE) -C coreinutils all

misc/life: libc/libc misc/life.c
	$(MAKE) -C misc

shell/sh: libc/libc shell/shell.c
	$(MAKE) -C shell

kernel/echidna.bin: $(KERNEL_ASM_FILES) $(KERNEL_C_FILES)
	$(MAKE) -C kernel

echidnafs/echfs-utils: echidnafs/echfs-utils.c
	cd echidnafs && gcc echfs-utils.c -o echfs-utils

clean:
	cd shell && $(MAKE) clean
	cd coreinutils && $(MAKE) clean
	cd libc && $(MAKE) clean
	cd misc && $(MAKE) clean
	cd kernel && $(MAKE) clean

echidna.img: echidnafs/echfs-utils bootloader/bootloader.asm kernel/echidna.bin shell/sh misc/life
	nasm bootloader/bootloader.asm -f bin -o echidna.img
	dd bs=512 count=131032 if=/dev/zero >> ./echidna.img
	echidnafs/echfs-utils echidna.img format
	echidnafs/echfs-utils echidna.img mkdir dev
	echidnafs/echfs-utils echidna.img mkdir bin
	echidnafs/echfs-utils echidna.img mkdir docs
	echidnafs/echfs-utils echidna.img import ./kernel/echidna.bin echidna.bin
	echidnafs/echfs-utils echidna.img import ./shell/sh /bin/sh
	echidnafs/echfs-utils echidna.img import ./misc/life /bin/life

	echidnafs/echfs-utils echidna.img import ./coreinutils/build/yes /bin/yes
	echidnafs/echfs-utils echidna.img import ./coreinutils/build/hello /bin/hello
	echidnafs/echfs-utils echidna.img import ./coreinutils/build/true /bin/true
	echidnafs/echfs-utils echidna.img import ./coreinutils/build/cp /bin/cp
	echidnafs/echfs-utils echidna.img import ./coreinutils/build/rm /bin/rm
	echidnafs/echfs-utils echidna.img import ./coreinutils/build/mv /bin/mv
	echidnafs/echfs-utils echidna.img import ./coreinutils/build/echo /bin/echo
	echidnafs/echfs-utils echidna.img import ./coreinutils/build/false /bin/false
#	echidnafs/echfs-utils echidna.img import ./coreinutils/build/[ /bin/[
	echidnafs/echfs-utils echidna.img import ./coreinutils/build/cat /bin/cat
	echidnafs/echfs-utils echidna.img import ./coreinutils/build/touch /bin/touch
	echidnafs/echfs-utils echidna.img import ./coreinutils/build/printf /bin/printf

	echidnafs/echfs-utils echidna.img import ./LICENSE.md /docs/license

clean-tools:
	rm -rf gcc-7.1.0 binutils-2.28 build-gcc build-binutils
	rm -f gcc-7.1.0.tar.bz2 binutils-2.28.tar.bz2

tools32: packages gcc-7.1.0 binutils-2.28
	rm -rf build-gcc/
	rm -rf build-binutils/
	export MAKEFLAGS="-j `grep -c ^processor /proc/cpuinfo`" && export PREFIX="`pwd`/tools" && export TARGET=i386-elf && export PATH="$$PREFIX/bin:$$PATH" && mkdir build-binutils && cd build-binutils && ../binutils-2.28/configure --target=$$TARGET --prefix="$$PREFIX" --with-sysroot --disable-nls --disable-werror && make && make install && cd ../gcc-7.1.0 && contrib/download_prerequisites && cd .. && mkdir build-gcc && cd build-gcc && ../gcc-7.1.0/configure --target=$$TARGET --prefix="$$PREFIX" --disable-nls --enable-languages=c --without-headers && make all-gcc && make all-target-libgcc && make install-gcc && make install-target-libgcc
	cp gccwrappers/* tools/bin/

tools64: packages gcc-7.1.0 binutils-2.28
	rm -rf build-gcc/
	rm -rf build-binutils/
	export MAKEFLAGS="-j `grep -c ^processor /proc/cpuinfo`" && export PREFIX="`pwd`/tools" && export TARGET=x86_64-elf && export PATH="$$PREFIX/bin:$$PATH" && mkdir build-binutils && cd build-binutils && ../binutils-2.28/configure --target=$$TARGET --prefix="$$PREFIX" --with-sysroot --disable-nls --disable-werror && make && make install && cd ../gcc-7.1.0 && contrib/download_prerequisites && cd .. && mkdir build-gcc && cd build-gcc && ../gcc-7.1.0/configure --target=$$TARGET --prefix="$$PREFIX" --disable-nls --enable-languages=c --without-headers && make all-gcc && make all-target-libgcc && make install-gcc && make install-target-libgcc
	cp gccwrappers/* tools/bin/

gcc-7.1.0.tar.bz2:
	wget ftp://ftp.gnu.org/gnu/gcc/gcc-7.1.0/gcc-7.1.0.tar.bz2

gcc-7.1.0: gcc-7.1.0.tar.bz2
	tar -vxf gcc-7.1.0.tar.bz2
	touch gcc-7.1.0

binutils-2.28.tar.bz2:
	wget ftp://ftp.gnu.org/gnu/binutils/binutils-2.28.tar.bz2

binutils-2.28: binutils-2.28.tar.bz2
	tar -vxf binutils-2.28.tar.bz2
	touch binutils-2.28

packages: sha256packages gcc-7.1.0.tar.bz2 binutils-2.28.tar.bz2
	sha256sum -c sha256packages
