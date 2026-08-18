all: loader

start.o:
	nasm -f elf32 start.s -o start.o

startup.o:
	nasm -f elf32 startup.s -o startup.o

loader.o: loader.c
	gcc -m32 -D_FILE_OFFSET_BITS=64 -c loader.c -o loader.o

loader: loader.o start.o startup.o
	ld -o loader loader.o startup.o start.o \
	-L/usr/lib32 \
	-lc \
	-T linking_script \
	-dynamic-linker /lib32/ld-linux.so.2

clean:
	rm -f *.o loader