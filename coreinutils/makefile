export PATH := ../tools/bin:$(PATH)
CC = cc
BINARY_NAMES = yes hello true cp rm mv echo false [ cat touch printf
BINARY_PATHS = $(addprefix build/,$(BINARY_NAMES))

notarget:
	$(error No target specified)

build/yes: ./src/yes.c
	$(CC) ./src/yes.c -o build/yes

build/hello: ./src/hello.c
	$(CC) ./src/hello.c -o build/hello

build/true: ./src/true.c
	$(CC) ./src/true.c -o build/true

build/cp: ./src/cp.c
	$(CC) ./src/cp.c ./src/_copy.c -o build/cp

build/rm: ./src/rm.c
	$(CC) ./src/rm.c -o build/rm

build/mv: ./src/mv.c
	$(CC) ./src/mv.c ./src/_copy.c -o build/mv

build/echo: ./src/echo.c
	$(CC) ./src/echo.c -o build/echo

build/false: ./src/false.c
	$(CC) ./src/false.c -o build/false

build/[: ./src/test.c
	$(CC) ./src/test.c -o build/[

build/cat: ./src/cat.c
	$(CC) ./src/cat.c -o build/cat

build/touch: ./src/touch.c
	$(CC) ./src/touch.c -o build/touch

build/printf: ./src/printf.c
	$(CC) ./src/printf.c -o build/printf

all: $(BINARY_PATHS)
	mkdir -p build
	touch coreinutils

clean:
	rm -rf build
