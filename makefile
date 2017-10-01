notarget:
	$(error No target specified)

distro: libc_target coreinutils_target shell/shell.bin misc/life.bin kernel/echidna.bin
	make img
	make clean

libc_target:
	cp gccwrappers/* tools/bin/
	export PATH=`pwd`/tools/bin:$$PATH && cd libc && make

coreinutils_target:
	export PATH=`pwd`/tools/bin:$$PATH && cd coreinutils && make all

misc/life.bin:
	export PATH=`pwd`/tools/bin:$$PATH && cd misc && make

shell/shell.bin:
	export PATH=`pwd`/tools/bin:$$PATH && cd shell && make

kernel/echidna.bin:
	export PATH=`pwd`/tools/bin:$$PATH && cd kernel && make

echidnafs/echfs-utils: echidnafs/echfs-utils.c
	cd echidnafs && gcc echfs-utils.c -o echfs-utils

clean:
	cd shell && make clean
	cd coreinutils && make clean
	cd libc && make clean
	cd misc && make clean
	cd kernel && make clean

img: echidnafs/echfs-utils kernel/echidna.bin
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
