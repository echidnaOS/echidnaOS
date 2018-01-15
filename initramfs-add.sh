#!/bin/bash

set -e

echidnafs/echfs-utils echidna.img format
echidnafs/echfs-utils echidna.img import kernel/echidna.bin echidna.bin
echidnafs/echfs-utils initramfs.img import $1 $2
echidnafs/echfs-utils echidna.img import initramfs.img initramfs.img
