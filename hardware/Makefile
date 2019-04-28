
CC=gcc
CPP=g++

#CPPFLAGS=-g -fpic -O0 -Wall -std=c++11
CPPFLAGS=-I. -I.. -g -fpic -O2 -Wall -Wextra -Werror -std=c++11
LDFLAGS=-g -shared -Wl,-soname,

LIBS= \
	cs2.so \
	m6051.so \
	rm485.so \
	virtual.so

%.so: %.cpp *.h ../manager.h
	$(CPP) $(CPPFLAGS) $(LDFLAGS)$@ -o $@ $<

all: $(LIBS)

clean:
	rm -f *.o *.so

