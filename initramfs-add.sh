#!/bin/bash

set -e
set -x

rm -f kernel/echidna.bin kernel/initramfs.c kernel/initramfs_obj.o
for i in "$@" ; do
    echidnafs/echfs-utils kernel/initramfs import $i $i
done
make -C kernel
echidnafs/echfs-utils echidna.img format 32768
echidnafs/echfs-utils echidna.img import kernel/echidna.bin echidna.bin
