KERNEL_FILES = $(shell find kernel -type f -name '*.c') $(shell find kernel -type f -name '*.asm') $(shell find kernel -type f -name '*.real') $(shell find kernel -type f -name '*.h')

notarget: echidna.img

misc/life: misc/life.c
	$(MAKE) -C misc

shell/sh: shell/shell.c
	$(MAKE) -C shell

kernel/echidna.bin: $(KERNEL_FILES)
	$(MAKE) -C kernel

echidnafs/echfs-utils: echidnafs/echfs-utils.c
	cd echidnafs && gcc echfs-utils.c -o echfs-utils

update_wrappers:
	cp gccwrappers/* tools/bin/

clean:
	cd echidnafs && rm -f echfs-utils
	$(MAKE) clean -C shell
	$(MAKE) clean -C misc
	$(MAKE) clean -C kernel

echidna.img: update_wrappers echidnafs/echfs-utils bootloader/bootloader.asm kernel/echidna.bin shell/sh misc/life
	nasm bootloader/bootloader.asm -f bin -o echidna.img
	dd bs=32768 count=8192 if=/dev/zero >> ./echidna.img
	truncate --size=-4096 echidna.img
	echidnafs/echfs-utils echidna.img format
	echidnafs/echfs-utils echidna.img mkdir dev
	echidnafs/echfs-utils echidna.img mkdir bin
	echidnafs/echfs-utils echidna.img mkdir docs
	echidnafs/echfs-utils echidna.img import ./kernel/echidna.bin echidna.bin
	echidnafs/echfs-utils echidna.img import ./shell/sh /bin/sh
	echidnafs/echfs-utils echidna.img import ./misc/life /bin/life
	echidnafs/echfs-utils echidna.img import ./LICENSE.md /docs/license
	rm tools/bin/kcc

clean-tools:
	rm -rf gcc-7.1.0 binutils-2.28 build-gcc build-binutils
	rm -f gcc-7.1.0.tar.bz2 binutils-2.28.tar.bz2

tools: packages gcc-7.1.0 binutils-2.28
	rm -rf build-gcc/
	rm -rf build-binutils/
	export MAKEFLAGS="-j `grep -c ^processor /proc/cpuinfo`" && export PREFIX="`pwd`/tools" && export TARGET=i386-elf && export PATH="$$PREFIX/bin:$$PATH" && mkdir build-binutils && cd build-binutils && ../binutils-2.28/configure --target=$$TARGET --prefix="$$PREFIX" --with-sysroot --disable-nls --disable-werror && make && make install && cd ../gcc-7.1.0 && contrib/download_prerequisites && cd .. && mkdir build-gcc && cd build-gcc && ../gcc-7.1.0/configure --target=$$TARGET --prefix="$$PREFIX" --disable-nls --enable-languages=c --without-headers && make all-gcc && make all-target-libgcc && make install-gcc && make install-target-libgcc
	git clone https://github.com/echidnaOS/echidnaOS-toolchain.git
	mkdir echidnaOS-toolchain/tools
	cp -rv tools/* echidnaOS-toolchain/tools/
	cp -rv gcc-7.1.0.tar.bz2 binutils-2.28.tar.bz2 echidnaOS-toolchain/
	cd echidnaOS-toolchain && ./maketools.sh
	cp echidnaOS-toolchain/newlib-patch/newlib/libc/sys/echidnaos/sys_api.h echidnaOS-toolchain/toolchain/usr/include/
	cp echidnaOS-toolchain/newlib-patch/newlib/libc/sys/echidnaos/sys_api.h echidnaOS-toolchain/toolchain/usr/i386-echidnaos/include/

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
	sha256sum -c sha256packages --ignore-missing
