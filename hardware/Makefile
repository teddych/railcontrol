
CC=g++

CPPFLAGS=-g -fpic -O2 -Wall -std=c++11
LDFLAGS=-g -shared -Wl,-soname,

LIBS= \
	cs2.so \
	virt.so

%.o: %.cpp %.h hardware.h
	$(CC) -c -o $@ $< $(CPPFLAGS)

%.so: %.o
	$(CC) -o $@ $< $(LDFLAGS)$@

all: $(LIBS)

clean:
	rm -f *.o *.so

