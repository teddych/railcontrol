
CC=g++

#CPPFLAGS=-g -fpic -O0 -Wall -std=c++11
CPPFLAGS=-I. -I.. -g -fpic -O2 -Wall -std=c++11
LDFLAGS=-g -shared -Wl,-soname,

LIBS= \
	cs2.so \
	virtual.so

%.so: %.cpp %.h hardware_interface.h
	$(CC) $(CPPFLAGS) $(LDFLAGS)$@ -o $@ $<

all: $(LIBS)

clean:
	rm -f *.o *.so

