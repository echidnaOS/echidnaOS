CC = cc
PREFIX = 

C_FILES = shell.c help.c echo.c cowsay.c
ASM_HEADER = header.asm
OBJ_FILES = header.o $(C_FILES:.c=.o)

CHARDFLAGS = -std=gnu99 -masm=intel
CFLAGS = 

shell.bin: $(OBJ_FILES)
	$(CC) -o shell.bin $(OBJ_FILES)

%.o: %.c
	$(CC) $(CHARDFLAGS) $(CFLAGS) -c $< -o $@

%.o: %.asm
	nasm $< -f elf32 -o $@

clean:
	rm $(OBJ_FILES) shell.bin

install:
	mkdir -p $(PREFIX)/bin
	cp shell.bin $(PREFIX)/bin/
