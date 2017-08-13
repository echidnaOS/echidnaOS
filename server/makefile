CC = ../tools/bin/i386-elf-gcc

C = server.c
OBJ = $(C:.c=.o)
LIBC = $(shell find ../libc/ -type f -name '*.o')

ifeq ($(LIBC),  )
$(error No libc found)
endif

CFLAGS = -std=gnu99 -ffreestanding -isystem ../libc/ -masm=intel -mpreferred-stack-boundary=2

shell.bin: $(OBJ) $(LIBC)
	$(CC) -T linker.ld -o server.bin -nostdlib $(OBJ) $(LIBC) -static-libgcc -lgcc -Xlinker --oformat=binary

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm $(OBJ)
