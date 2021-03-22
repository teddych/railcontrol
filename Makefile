
RAILCONTROL_VERSION := 19

CFLAGSSQLITE=-g -O2 -DSQLITE_ENABLE_FTS4 -DSQLITE_ENABLE_JSON1 -DSQLITE_ENABLE_RTREE -DHAVE_USLEEP
CFLAGSZLIB=-g -O2 -Wno-implicit-function-declaration
CXXFLAGS=-I. -g -O2 -Wall -Wextra -pedantic -Werror -std=c++11
CXXFLAGSAMALGAMATION=-I. -g -O2 -Wall -Wextra -Werror -std=c++11
LDFLAGS=-g
LIBS=-lpthread -ldl
LIBSAMALGAMATION=-lpthread -ldl

ifeq ($(OS),Windows_NT)
	LDFLAGS+= -static
endif

TMPDIR=/tmp/RailControl
TMPDIRCYGWIN=/RailControl

CXXOBJ= $(patsubst %.cpp,%.o,$(sort Version.cpp $(wildcard *.cpp)) $(wildcard WebServer/*.cpp DataModel/*.cpp Hardware/*.cpp Logger/*.cpp Network/*.cpp Storage/*.cpp Utils/*.cpp))
COBJ= $(patsubst %.c,%.o,$(wildcard Hardware/zlib/*.c))
OBJ=Storage/sqlite/sqlite3.o $(CXXOBJ) $(COBJ)

all: $(OBJ)
	$(CXX) $(LDFLAGS) $(OBJ) -o railcontrol $(LIBS)

dist: all
	strip railcontrol
	mkdir $(TMPDIR)
	cp -r \
		html \
		railcontrol.conf.dist \
		railcontrol \
		$(TMPDIR)
	( cd $(TMPDIR)/.. && tar cvJf railcontrol.`date +"%Y%m%d"`.tar.xz RailControl/* RailControl/html/* )
	rm -r $(TMPDIR)

dist-cygwin: all
	strip railcontrol.exe
	mkdir $(TMPDIRCYGWIN)
	cp -r \
		/cygdrive/c/Windows/SYSTEM32/ntdll.dll \
		/cygdrive/c/Windows/system32/KERNELBASE.dll \
		/cygdrive/c/Windows/system32/kernel32.dll \
		/usr/bin/cygwin1.dll \
		html \
		railcontrol.conf.dist \
		railcontrol.exe \
		$(TMPDIRCYGWIN)
	zip -9 railcontrol.windows.`date +"%Y%m%d"`.zip $(TMPDIRCYGWIN)/* $(TMPDIRCYGWIN)/html/*
	rm -r $(TMPDIRCYGWIN)

amalgamation: Version.cpp
	./amalgamation.bash
	$(CXX) $(CXXFLAGSAMALGAMATION) -DAMALGAMATION -c -o amalgamation.o amalgamation.cpp
	$(CXX) -g amalgamation.o Storage/sqlite/sqlite3.o Hardware/zlib/*.o -o railcontrol $(LIBSAMALGAMATION)
	rm -f amalgamation.o
	rm -f amalgamation.cpp

sqlite-shell:
	make -C Storage/sqlite

Version.o: Version.cpp Version.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

Hardware/zlib/%.o: Hardware/zlib/%.c Hardware/zlib/*.h
	$(CC) $(CFLAGSZLIB) -c -o $@ $<

Storage/sqlite/sqlite3.o: Storage/sqlite/sqlite3.c Storage/sqlite/sqlite3.h
	$(CC) $(CFLAGSSQLITE) -c -o $@ $<

%.o: %.cpp *.h DataModel/*.h Hardware/*.h Logger/*.h Network/*.h Storage/*.h Utils/*.h WebServer/*.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm -f *.o DataModel/*.o Hardware/*.o Hardware/zlib/*.o Logger/*.o Network/*.o Storage/*.o Storage/sqlite/*.o Utils/*.o WebServer/*.o
	rm -f railcontrol

clean-sqlite-shell:
	make -C Storage/sqlite clean

test:
	make -C test

tools:
	make -C tools

.PHONY: Version.cpp
Version.cpp: Version.cpp.in
	sed s/@COMPILE_TIMESTAMP@/`date +%s`/ < Version.cpp.in \
	| sed s/@GIT_HASH@/`git log -1 --format=%H`/ \
	| sed s/@GIT_TIMESTAMP@/`git log -1 --format=%at`/ \
	| sed s/@GIT_DIRTY@/`git status -s| wc -l`/ \
	| sed s/@RAILCONTROL_VERSION@/$(RAILCONTROL_VERSION)/ \
	> Version.cpp

