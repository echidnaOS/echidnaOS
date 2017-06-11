ARCH = i386

ifeq ($(ARCH), i386)
CDEF = -D__IA32__
else ifeq ($(ARCH), i686)
CDEF = -D__IA32__
else ifeq ($(ARCH), x86_64)
CDEF = -D__AMD64__
else
$(error ARCH variable invalid, available architectures: i386, i686, x86_64)
endif

CC = ../gcc/bin/$(ARCH)-elf-gcc
LD = ../gcc/bin/$(ARCH)-elf-ld

C = $(shell find ./ -type f,l -name '*.c')
OBJ = $(C:.c=.o)
LIBC = $(shell find ../libc/ -type f,l -name '*.o')

ifeq ($(LIBC),  )
$(error No libc found)
endif

CFLAGS = -std=gnu99 -ffreestanding -isystem ../libc/ -masm=intel $(CDEF)

shell.bin: $(OBJ) $(LIBC)
	$(LD) -T linker.ld --oformat binary -o shell.bin -nostdlib $(OBJ) $(LIBC)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm $(OBJ)
