CC = ../gcc/bin/i386-elf-gcc
LD = ../gcc/bin/i386-elf-ld

C = shell.c
OBJ = $(C:.c=.o)
LIBC = $(shell find ../libc/ -type f,l -name '*.o')

ifeq ($(LIBC),  )
$(error No libc found)
endif

CFLAGS = -std=gnu99 -ffreestanding -isystem ../libc/ -masm=intel

shell.bin: $(OBJ) $(LIBC)
	$(LD) -T linker.ld --oformat binary -o shell.bin -nostdlib $(OBJ) $(LIBC)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm $(OBJ)
