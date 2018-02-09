PATH := $(PATH):$(shell pwd)/echidnaOS-toolchain/toolchain/bin

notarget: echidna.img

shell_target:
	$(MAKE) -C shell

kernel_target: kernel/initramfs
	$(MAKE) -C kernel

echidnafs/echfs-utils: echidnafs/echfs-utils.c
	cd echidnafs && gcc -O2 echfs-utils.c -o echfs-utils

clean:
	cd echidnafs && rm -f echfs-utils
	rm -f kernel/initramfs
	$(MAKE) clean -C shell
	$(MAKE) clean -C kernel

kernel/initramfs: echidnafs/echfs-utils shell_target
	dd bs=32768 count=256 if=/dev/zero of=kernel/initramfs
	echidnafs/echfs-utils kernel/initramfs format 32768
	echidnafs/echfs-utils kernel/initramfs mkdir dev
	echidnafs/echfs-utils kernel/initramfs mkdir bin
	echidnafs/echfs-utils kernel/initramfs mkdir sys
	echidnafs/echfs-utils kernel/initramfs mkdir docs
	echidnafs/echfs-utils kernel/initramfs mkdir tmp
	echidnafs/echfs-utils kernel/initramfs mkdir mnt
# aa
	echidnafs/echfs-utils kernel/initramfs import echidnaOS-toolchain/native/lib/gcc/i386-echidnaos/7.1.0/specs home/mint/repos/echidnaOS/echidnaOS-toolchain/native/lib/gcc/i386-echidnaos/specs
# bb
	echidnafs/echfs-utils kernel/initramfs import ./shell/sh /sys/init
	echidnafs/echfs-utils kernel/initramfs import ./LICENSE.md /docs/license

echidna.img: echidnafs/echfs-utils kernel_target
	nasm bootloader/bootloader.asm -f bin -o echidna.img
	dd bs=32768 count=16384 if=/dev/zero >> ./echidna.img
	truncate --size=-4096 echidna.img
	echidnafs/echfs-utils echidna.img format 32768
	echidnafs/echfs-utils echidna.img import ./kernel/echidna.bin echidna.bin

echidnaOS-toolchain:
	git clone https://github.com/echidnaOS/echidnaOS-toolchain.git

tools: echidnaOS-toolchain
	cd echidnaOS-toolchain && ./maketoolchain.sh
