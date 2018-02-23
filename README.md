# echidnaOS

## What is echidnaOS?

echidnaOS is a 64 bit operating system written from scratch.
The kernel is written primarily in C, with some assembly here and there.

# Building Instructions

Please, follow these instructions to build a bootable ISO of echidnaOS.

## Requirements

* nasm
* gnu make
* gcc
* g++
* automake
* autoconf
* xorriso
* grub
* qemu (for testing the image, non essential)

## Step by step:

You can install all the above packages on Ubuntu/Debian with the following
commands:
```
sudo apt-get update
sudo apt-get install nasm build-essential automake autoconf qemu-system-x86 xorriso
```
Make sure you are in the root of the source tree.

Now let's build the cross toolchain:
```
make tools
```
This step will take a while, especially on slower systems.

Now that every requirement is satisfied, let's build the kernel, the shell,
and create the image:
```
make iso
```
If make didn't error, congratulations, you managed to build echidnaOS.
There should be a bootable "echidna.iso" image in the project's root now.

You can test the image in qemu with:
```
qemu-system-x86_64 echidna.iso
```
