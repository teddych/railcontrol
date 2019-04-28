
CC=gcc
CPP=g++
CCRASPI=aarch64-linux-gcc
CPPRASPI=aarch64-linux-g++

CFLAGS=-g -fpic -O2 -Wall -Wextra -Werror
CFLAGSAMALGAMATION=-g -O2 -Wall -Wextra -Werror
CFLAGSRASPI=-g -O2 -Wall --sysroot=/home/teddy/buildroot-2018.11/output/host/aarch64-buildroot-linux-gnu/sysroot/
CPPFLAGS=-g -fpic -O2 -Wall -Wextra -Werror -std=c++11 -I..
CPPFLAGSRASPI=-g -fpic -O2 -Wall --Wextra -Werror std=c++11 -I.. --sysroot=/home/teddy/buildroot-2018.11/output/host/aarch64-buildroot-linux-gnu/sysroot/
LDFLAGS=-g -shared -Wl,-soname,
CFLAGSSQLITE=-DSQLITE_ENABLE_FTS4 -DSQLITE_ENABLE_JSON1 -DSQLITE_ENABLE_RTREE -DHAVE_USLEEP

LIBS= \
	sqlite.so

all: $(LIBS)

sqlite.so: sqlite.o sqlite/sqlite3.o *.h ../Logger/*.h ../hardware/HardwareParams.h ../datamodel/*.h
	$(CPP) $(CPPFLAGS) $(LDFLAGS)$@ -o $@ $<  sqlite/sqlite3.o

sqlite/sqlite3.o: sqlite/sqlite3.c sqlite/sqlite3.h
	$(CC) $(CFLAGS) $(CFLAGSSQLITE) -c -o $@ $<

amalgamation: sqlite/sqlite3.c sqlite/sqlite3.h
	$(CC) $(CFLAGSAMALGAMATION) $(CFLAGSSQLITE) -c -o sqlite/sqlite3.o sqlite/sqlite3.c

raspi: sqlite/sqlite3.c sqlite/sqlite3.h
	$(CCRASPI) $(CFLAGSRASPI) $(CFLAGSSQLITE) -c -o sqlite/sqlite3.o sqlite/sqlite3.c

%.so: %.cpp *.h ../*.h ../datamodel/*.h
	$(CPP) $(CPPFLAGS) $(LDFLAGS)$@ -o $@ $<

%.so: %.o *.h ../*.h ../datamodel/*.h
	$(CPP) -o $@ $< $(LDFLAGS)$@

clean:
	rm -f *.o *.so sqlite/*.o

