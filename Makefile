CC = gcc
CFLAGS = -g -O2 -Wall -Wno-unused-result -fPIC -shared 
LDFLAGS = -ldl

all: memshim64 memshim32

memshim64: memshim.c
	$(CC) $(CFLAGS) -o memshim64.so memshim.c $(LDFLAGS) 

memshim32: memshim.c
	$(CC) -m32 $(CFLAGS) -o memshim32.so memshim.c $(LDFLAGS)

clean:
	rm -f *.so *.o
