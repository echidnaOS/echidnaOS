export PATH := ../tools/bin:$(PATH)
CC = cc
C_FILES = $(shell find ./src/ -type f -name '*.c')

notarget:
	$(error No target specified)

yes: ./src/yes.c
	$(CC) ./src/yes.c -o yes
	mv yes ./build/yes

hello: ./src/hello.c
	$(CC) ./src/hello.c -o hello
	mv hello ./build/hello

true: ./src/true.c
	$(CC) ./src/true.c -o true
	mv true ./build/true

cp: ./src/cp.c
	$(CC) ./src/cp.c ./src/_copy.c -o cp
	mv cp ./build/cp

rm: ./src/rm.c
	$(CC) ./src/rm.c -o rm
	mv rm ./build/rm

mv: ./src/mv.c
	$(CC) ./src/mv.c ./src/_copy.c -o mv
	mv mv ./build/mv

echo: ./src/echo.c
	$(CC) ./src/echo.c -o echo
	mv echo ./build/echo

false: ./src/false.c
	$(CC) ./src/false.c -o false
	mv false ./build/false

test: ./src/test.c
	$(CC) ./src/test.c -o [
	mv [ ./build/[

cat: ./src/cat.c
	$(CC) ./src/cat.c -o cat
	mv cat ./build/cat

touch: ./src/touch.c
	$(CC) ./src/touch.c -o touch
	mv touch ./build/touch

printf: ./src/printf.c
	$(CC) ./src/printf.c -o printf
	mv printf ./build/printf

all: $(C_FILES)
	mkdir build 2> /dev/null || true
	make yes
	make hello
	make true
	make cp
	make rm
	make mv
	make echo
	make false
#	make test
	make cat
	make touch
	make printf

clean:
	rm -rf build
