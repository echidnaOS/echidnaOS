export PATH := ../echidnaOS-toolchain/toolchain/bin:$(PATH)
CC = i386-echidnaos-gcc
PREFIX = 

CFLAGS = -O2

life: life.c
	$(CC) $(CFLAGS) life.c -o life

clean:
	rm -f life
