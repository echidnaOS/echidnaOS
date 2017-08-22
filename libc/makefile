CC = cc
PREFIX = /usr

C_FILES = $(shell find ./ -type f -name '*.c')
OBJ_FILES = $(C_FILES:.c=.o)

CHARDFLAGS = -std=gnu99 -masm=intel
CFLAGS = 

libc: $(OBJ_FILES)
	touch libc

%.o: %.c
	$(CC) $(CHARDFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm $(OBJ_FILES)

install:
	mkdir -p $(PREFIX)/lib
	cp $(OBJ_FILES) $(PREFIX)/lib/
