CC = ../tools/bin/i386-elf-gcc

C_FILES = init.c $(shell find ./src/ -type f -name '*.c')
OBJ_FILES = $(C_FILES:.c=.o)
LIBC = $(shell find ../libc/ -type f -name '*.o')

ifeq ($(LIBC),  )
$(error No libc found)
endif

CFLAGS = -std=gnu99 -ffreestanding -isystem ../libc/ -masm=intel

shell.bin: $(OBJ_FILES) $(LIBC)
	$(CC) -T linker.ld -o vfs.bin -nostdlib $(OBJ_FILES) $(LIBC) -static-libgcc -lgcc -Xlinker --oformat=binary

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm $(OBJ_FILES)
