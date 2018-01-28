#!/bin/bash

set -e
set -x

cp gccwrappers/* tools/bin/
rm -f kernel/echidna.bin kernel/initramfs.c kernel/initramfs_obj.o
for i in "$@" ; do
    echidnafs/echfs-utils kernel/initramfs import $i $i
done
make -C kernel
echidnafs/echfs-utils echidna.img format
echidnafs/echfs-utils echidna.img import kernel/echidna.bin echidna.bin
rm tools/bin/kcc
