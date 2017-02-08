
CC=gcc
CPP=g++

CFLAGS=-g -fpic -O2 -Wall
CPPFLAGS=-g -fpic -O2 -Wall -std=c++11 -I..
LDFLAGS=-g -shared -Wl,-soname,

LIBS= \
	sqlite.so

all: $(LIBS)

sqlite.so: sqlite.o sqlite/sqlite3.o
	$(CPP) $(CPPFLAGS) $(LDFLAGS)$@ -o $@ $<  sqlite/sqlite3.o

sqlite/sqlite3.o: sqlite/sqlite3.c
	$(CC) $(CFLAGS) \
	-DSQLITE_ENABLE_FTS4 \
	-DSQLITE_ENABLE_JSON1 \
	-DSQLITE_ENABLE_RTREE \
	-DHAVE_USLEEP \
	-c -o $@ $<

#sqlite.so: sqlite.cpp
#	$(CPP) $(CPPFLAGS) $(LDFLAGS)$@ -o $@ $< sqlite/sqlite3.o -ldl -pthread

%.so: %.cpp %.h
	$(CPP) $(CPPFLAGS) $(LDFLAGS)$@ -o $@ $<

%.so: %.o
	$(CPP) -o $@ $< $(LDFLAGS)$@

clean:
	rm -f *.o *.so

