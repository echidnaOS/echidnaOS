CC = cc
PREFIX = 

C_FILES = shell.c
OBJ_FILES = $(C_FILES:.c=.o)

CHARDFLAGS = -std=gnu99 -masm=intel
CFLAGS = 

sh: $(OBJ_FILES)
	$(CC) -o sh $(OBJ_FILES)

%.o: %.c
	$(CC) $(CHARDFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm $(OBJ_FILES) sh

install:
	mkdir -p $(PREFIX)/bin
	cp sh $(PREFIX)/bin/
