
CC=g++

#CPPFLAGS=-g -fpic -O0 -Wall -std=c++11
CPPFLAGS=-I. -I.. -g -fpic -O2 -Wall -std=c++11
LDFLAGS=-g -shared -Wl,-soname,

LIBS= \
	CS2.so \
	Virtual.so

%.so: %.cpp *.h ../*.h ../datamodel/*.h
	$(CC) $(CPPFLAGS) $(LDFLAGS)$@ -o $@ $<

all: $(LIBS)

clean:
	rm -f *.o *.so

