
CC=gcc
CPP=g++
CCRASPI=aarch64-linux-gcc
CPPRASPI=aarch64-linux-g++

CFLAGS=-g -fpic -O2 -Wall
CFLAGSAMALGAMATION=-g -O2 -Wall
CFLAGSRASPI=-g -O2 -Wall --sysroot=/home/teddy/buildroot-2018.11/output/host/aarch64-buildroot-linux-uclibc/sysroot/
CPPFLAGS=-g -fpic -O2 -Wall -std=c++11 -I..
CPPFLAGSRASPI=-g -fpic -O2 -Wall -std=c++11 -I.. --sysroot=/home/teddy/buildroot-2018.11/output/host/aarch64-buildroot-linux-uclibc/sysroot/
LDFLAGS=-g -shared -Wl,-soname,

LIBS= \
	sqlite.so

all: $(LIBS)

sqlite.so: sqlite.o sqlite/sqlite3.o *.h ../Logger/*.h ../hardware/HardwareParams.h ../datamodel/*.h
	$(CPP) $(CPPFLAGS) $(LDFLAGS)$@ -o $@ $<  sqlite/sqlite3.o

sqlite/sqlite3.o: sqlite/sqlite3.c sqlite/sqlite3.h
	$(CC) $(CFLAGS) \
	-DSQLITE_ENABLE_FTS4 \
	-DSQLITE_ENABLE_JSON1 \
	-DSQLITE_ENABLE_RTREE \
	-DHAVE_USLEEP \
	-c -o $@ $<

amalgamation: sqlite/sqlite3.c sqlite/sqlite3.h
	$(CC) $(CFLAGSAMALGAMATION) \
	-DSQLITE_ENABLE_FTS4 \
	-DSQLITE_ENABLE_JSON1 \
	-DSQLITE_ENABLE_RTREE \
	-DHAVE_USLEEP \
	-c -o sqlite/sqlite3.o sqlite/sqlite3.c

raspi: sqlite/sqlite3.c sqlite/sqlite3.h
	$(CCRASPI) $(CFLAGSRASPI) \
	-DSQLITE_ENABLE_FTS4 \
	-DSQLITE_ENABLE_JSON1 \
	-DSQLITE_ENABLE_RTREE \
	-DHAVE_USLEEP \
	-c -o sqlite/sqlite3.o sqlite/sqlite3.c

%.so: %.cpp *.h ../*.h ../datamodel/*.h
	$(CPP) $(CPPFLAGS) $(LDFLAGS)$@ -o $@ $<

%.so: %.o *.h ../*.h ../datamodel/*.h
	$(CPP) -o $@ $< $(LDFLAGS)$@

clean:
	rm -f *.o *.so

