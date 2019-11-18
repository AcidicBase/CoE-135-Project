CFLAGS = -Wall

.PHONY: all clean tests

all: Colendres_MP
example: Colendres_MP.o murmur3.o

shared: murmur3.c murmur3.h
	$(CC) -fPIC -O3 -c murmur3.c
	$(CC) -shared -Wl,--export-dynamic murmur3.o -o libmurmur3.so

clean:
	rm -rf Colendres_MP *.o *.so
