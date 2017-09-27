CC = cc
PREFIX = 

CHARDFLAGS = -std=gnu99 -masm=intel
CFLAGS = 

check: check.c
	$(CC) $(CHARDFLAGS) -o check.o -c check.c
	$(CC) -o check check.o

clean:
	rm check.o check
