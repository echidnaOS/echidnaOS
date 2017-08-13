CC = ../tools/bin/i386-elf-gcc

C_FILES = shell.c help.c echo.c cowsay.c
ASM_HEADER = header.asm
OBJ_FILES = header.o $(C_FILES:.c=.o)
LIBC = $(shell find ../libc/ -type f -name '*.o')

ifeq ($(LIBC),  )
$(error No libc found)
endif

CFLAGS = -std=gnu99 -ffreestanding -isystem ../libc/ -masm=intel -mpreferred-stack-boundary=2

shell.bin: $(OBJ_FILES) $(LIBC)
	$(CC) -T linker.ld -o shell.bin -nostdlib $(OBJ_FILES) $(LIBC) -static-libgcc -lgcc -Xlinker --oformat=binary

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	nasm $< -f elf32 -o $@

.PHONY: clean

clean:
	rm $(OBJ_FILES)
