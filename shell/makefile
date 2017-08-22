CC = cc
PREFIX = 

C_FILES = shell.c help.c echo.c cowsay.c
OBJ_FILES = $(C_FILES:.c=.o)

CHARDFLAGS = -std=gnu99 -masm=intel
CFLAGS = 

shell.bin: $(OBJ_FILES)
	$(CC) -o shell.bin $(OBJ_FILES)

%.o: %.c
	$(CC) $(CHARDFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm $(OBJ_FILES) shell.bin

install:
	mkdir -p $(PREFIX)/bin
	cp shell.bin $(PREFIX)/bin/
