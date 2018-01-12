export PATH := ../echidnaOS-toolchain/toolchain/bin:$(PATH)
CC = i386-echidnaos-gcc
PREFIX = 

CFLAGS = -O3 -Wall

echo: echo.c
	$(CC) $(CFLAGS) echo.c -o echo

clean:
	rm -f echo
