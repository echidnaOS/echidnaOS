export PATH := ../tools/bin:$(PATH)
CC = cc
BINARY_NAMES = yes hello true cp rm mv echo false [ cat touch printf
BINARY_PATHS = $(addprefix build/,$(BINARY_NAMES))

CHARDFLAGS = -masm=intel
CFLAGS = -O1

coreinutils: $(BINARY_PATHS)
	touch coreinutils

build/yes: ./src/coreinutils.h ./src/yes.c
	mkdir -p build
	$(CC) $(CHARDFLAGS) $(CFLAGS) ./src/yes.c -o build/yes

build/hello: ./src/coreinutils.h ./src/hello.c
	mkdir -p build
	$(CC) $(CHARDFLAGS) $(CFLAGS) ./src/hello.c -o build/hello

build/true: ./src/coreinutils.h ./src/true.c
	mkdir -p build
	$(CC) $(CHARDFLAGS) $(CFLAGS) ./src/true.c -o build/true

build/cp: ./src/coreinutils.h ./src/cp.c ./src/_copy.c
	mkdir -p build
	$(CC) $(CHARDFLAGS) $(CFLAGS) ./src/cp.c ./src/_copy.c -o build/cp

build/rm: ./src/coreinutils.h ./src/rm.c
	mkdir -p build
	$(CC) $(CHARDFLAGS) $(CFLAGS) ./src/rm.c -o build/rm

build/mv: ./src/coreinutils.h ./src/mv.c ./src/_copy.c
	mkdir -p build
	$(CC) $(CHARDFLAGS) $(CFLAGS) ./src/mv.c ./src/_copy.c -o build/mv

build/echo: ./src/coreinutils.h ./src/echo.c
	mkdir -p build
	$(CC) $(CHARDFLAGS) $(CFLAGS) ./src/echo.c -o build/echo

build/false: ./src/coreinutils.h ./src/false.c
	mkdir -p build
	$(CC) $(CHARDFLAGS) $(CFLAGS) ./src/false.c -o build/false

build/[: ./src/coreinutils.h ./src/test.c
	mkdir -p build
	$(CC) $(CHARDFLAGS) $(CFLAGS) ./src/test.c -o build/[

build/cat: ./src/coreinutils.h ./src/cat.c
	mkdir -p build
	$(CC) $(CHARDFLAGS) $(CFLAGS) ./src/cat.c -o build/cat

build/touch: ./src/coreinutils.h ./src/touch.c
	mkdir -p build
	$(CC) $(CHARDFLAGS) $(CFLAGS) ./src/touch.c -o build/touch

build/printf: ./src/coreinutils.h ./src/printf.c
	mkdir -p build
	$(CC) $(CHARDFLAGS) $(CFLAGS) ./src/printf.c -o build/printf

clean:
	rm -rf build coreinutils
