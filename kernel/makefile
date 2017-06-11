ARCH = i386

ifeq ($(ARCH), i386)
CDEF = -D__IA32__
NASMFLAGS = -f elf32
else ifeq ($(ARCH), i686)
CDEF = -D__IA32__
NASMFLAGS = -f elf32
else ifeq ($(ARCH), x86_64)
CDEF = -D__AMD64__
NASMFLAGS = -f elf64
else
$(error ARCH variable invalid, available architectures: i386, i686, x86_64)
endif

CC = ../gcc/bin/$(ARCH)-elf-gcc
LD = ../gcc/bin/$(ARCH)-elf-ld

C_FILES = ./init.c $(shell find ./common/ -type f,l -name '*.c')
H_FILES = $(shell find ./ -type f,l -name '*.h')
ASM_FILES = $(shell find ./$(ARCH)/ -type f,l -name '*.asm')
REAL_FILES = $(shell find ./blobs/ -type f,l -name '*.real')
OBJ = $(C_FILES:.c=.o) $(ASM_FILES:.asm=.o)
BINS = $(REAL_FILES:.real=.bin)

CFLAGS = -std=gnu99 -ffreestanding -isystem ./include -masm=intel $(CDEF)

echidna.bin: ../shell/shell.bin $(BINS) $(OBJ)
	$(LD) -T linker.ld --oformat binary -o echidna.bin -nostdlib $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.bin: %.real
	nasm $< -f bin -o $@

%.o: %.asm
	nasm $< $(NASMFLAGS) -o $@

../shell/shell.bin:
	$(error Unable to find shell.bin)

.PHONY: clean

clean:
	rm $(OBJ) $(BINS)
