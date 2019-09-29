
CC=gcc
CPP=g++

#CPPFLAGS=-g -fpic -O0 -Wall -std=c++11
CPPFLAGS=-I. -I.. -g -fpic -O2 -Wall -Wextra -Werror -std=c++11
LDFLAGS=-g -shared -Wl,-soname,

LIBS= \
	CS2.so \
	M6051.so \
	OpenDcc.so \
	RM485.so \
	Virtual.so

%.so: %.cpp *.h ../Manager.h
	$(CPP) $(CPPFLAGS) $(LDFLAGS)$@ -o $@ $<

all: $(LIBS)

clean:
	rm -f *.o *.so

