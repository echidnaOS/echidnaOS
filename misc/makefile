export PATH := ../tools/bin:$(PATH)
CC = cc
PREFIX = 

CHARDFLAGS = -std=gnu99 -masm=intel
CFLAGS = 

life: life.c
	$(CC) $(CHARDFLAGS) -o life.o -c life.c
	$(CC) -o life life.o

clean:
	rm -f life.o life
