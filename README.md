# echidnaOS

## What is echidnaOS?

echidnaOS is an attempt at creating an operating system from scratch.
The kernel is written primarily in C, with some assembly here and there.

# Building Instructions

Please, follow these instructions to build a bootable IMG of echidnaOS.
You can then write the resulting image to a Hard Drive.

## Requirements

* nasm
* gnu make
* gcc
* g++
* bison
* qemu (for testing the image, non essential)

## Step by step:

You can install all the above packages on Ubuntu/Debian with the following
commands:
```
sudo apt-get update
sudo apt-get install nasm build-essential qemu-system-x86
```
Make sure you are in the root of the source tree.

Now let's build the cross toolchain:
```
./maketools.sh
```
This step will take a while, especially on slower systems.

You can now use:
```
make clean-tools
```
To remove build files and sources that aren't needed anymore.

Now that every requirement is satisfied, let's build the kernel, the shell,
and create the image:
```
make
```
If make didn't error, congratulations, you managed to build echidnaOS.
There should be a bootable "echidna.img" image in the project's root now.

You can test the image in qemu with:
```
qemu-system-i386 echidna.img
```
