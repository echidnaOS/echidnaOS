export PATH := ../tools/bin:$(PATH)
CC = cc
PREFIX = 

CHARDFLAGS = -std=gnu99 -masm=intel
CFLAGS = 

life: life.c
	$(CC) $(CHARDFLAGS) $(CFLAGS) life.c -o life

clean:
	rm -f life
