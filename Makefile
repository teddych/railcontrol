ifeq ("$(wildcard .git)", "")
RAILCONTROL_VERSION=$(shell cat VERSION_RAILCONTROL | sed s/v//)
GIT_HASH=$(shell cat VERSION_GIT_HASH)
GIT_TIMESTAMP=$(shell cat VERSION_GIT_TIMESTAMP)
GIT_DIRTY=0
else
.PHONY: Version.cpp
RAILCONTROL_VERSION=$(shell git describe --tags --abbrev=0 | sed s/v//)
GIT_HASH=$(shell git log -1 --format=%H)
GIT_TIMESTAMP=$(shell git log -1 --format=%at)
GIT_DIRTY=$(shell git status -s | wc -l)
endif

CFLAGSSQLITE=-g -O2 -DSQLITE_ENABLE_FTS4 -DSQLITE_ENABLE_JSON1 -DSQLITE_ENABLE_RTREE -DHAVE_USLEEP
CFLAGSZLIB=-g -O2 -Wno-implicit-function-declaration
CXXFLAGS=-I. -g -O2 -Wall -Wextra -pedantic -Werror -Wno-missing-braces -std=c++11 -D_GNU_SOURCE
CXXFLAGSAMALGAMATION=-I. -g -O2 -Wall -Wextra -pedantic -Werror -Wno-missing-braces -std=c++11
LDFLAGS=-g
LIBS=-lpthread -ldl
LIBSAMALGAMATION=-lpthread -ldl

ifeq ($(OS),Windows_NT)
	LDFLAGS+= -static
endif

TMPDIR=/tmp/RailControl
TMPDIRCYGWIN=/RailControl

CXXOBJ= $(patsubst %.cpp,%.o,$(sort Version.cpp $(wildcard *.cpp)) $(wildcard Server/Web/*.cpp Server/CS2/*.cpp Server/Z21/*.cpp DataModel/*.cpp Hardware/*.cpp Hardware/Protocols/*.cpp Logger/*.cpp Network/*.cpp Storage/*.cpp Utils/*.cpp))

ifdef USE_SYSTEM_LIBRARIES
LIBS+= -lsqlite3 -lz
OBJ=$(CXXOBJ)
else
CXXFLAGS+= -IStorage/sqlite -IHardware/zlib
COBJ= $(patsubst %.c,%.o,$(wildcard Hardware/zlib/*.c))
OBJ=Storage/sqlite/sqlite3.o $(CXXOBJ) $(COBJ)
endif

SOURCE_DATE_EPOCH?=$(shell date +%s)

all: railcontrol

railcontrol: $(OBJ)
	$(CXX) $(LDFLAGS) $(OBJ) -o $@ $(LIBS)

noupdatecheck: CXXFLAGS += -DNOUPDATECHECK
noupdatecheck: railcontrol

strip: railcontrol
	strip railcontrol

dist: railcontrol
	strip railcontrol
	mkdir $(TMPDIR)
	cp -r \
		html \
		railcontrol.conf.dist \
		railcontrol \
		$(TMPDIR)
	( cd $(TMPDIR)/.. && tar cvJf railcontrol.`date +"%Y%m%d"`.tar.xz RailControl/* )
	rm -r $(TMPDIR)

dist-cygwin: railcontrol
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

amalgamation.cpp:
	./amalgamation.bash

amalgamation: amalgamation.o Version.cpp Storage/sqlite/sqlite3.o $(COBJ)
	$(CXX) -g amalgamation.o Storage/sqlite/sqlite3.o Hardware/zlib/*.o -o railcontrol $(LIBSAMALGAMATION)
	strip railcontrol
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

%.o: %.cpp *.h DataModel/*.h Hardware/*.h Hardware/Protocols/*.h Logger/*.h Network/*.h Storage/*.h Utils/*.h Server/Web/*.h Server/CS2/*.h Server/Z21/*.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm -f *.o DataModel/*.o Hardware/*.o Hardware/Protocols/*.o Hardware/zlib/*.o Logger/*.o Network/*.o Storage/*.o Storage/sqlite/*.o Utils/*.o Server/Web/*.o Server/CS2/*.o Server/Z21/*.o
	rm -f railcontrol
	rm -f Documentation/de/index.html Documentation/en/index.html Documentation/es/index.html

clean-sqlite-shell:
	make -C Storage/sqlite clean

test: railcontrol
	pytest -vv

tools:
	make -C tools

Version.cpp: Version.cpp.in VERSION_RAILCONTROL VERSION_GIT_HASH VERSION_GIT_TIMESTAMP
	sed -e s/@COMPILE_TIMESTAMP@/$(SOURCE_DATE_EPOCH)/ \
	    -e s/@GIT_HASH@/$(GIT_HASH)/ \
	    -e s/@GIT_TIMESTAMP@/$(GIT_TIMESTAMP)/ \
	    -e "s/@GIT_DIRTY@/$(GIT_DIRTY)/" \
	    -e s/@RAILCONTROL_VERSION@/$(RAILCONTROL_VERSION)/ \
	    < $< > $@

doc:
	pandoc --toc --toc-depth=2 --standalone --css=../style.css \
		--template=Documentation/template.html \
		--metadata-file=Documentation/de/metadata.yaml \
		Documentation/de/index.md \
		Documentation/de/startup-arguments.md \
		Documentation/de/general-settings.md \
		Documentation/de/config-controls.md \
		Documentation/de/config-intellibox.md \
		Documentation/de/config-mastercontrol2.md \
		Documentation/de/config-mastercontrol.md \
		Documentation/de/config-redbox.md \
		Documentation/de/config-twincenter.md \
		Documentation/de/control-6051.md \
		Documentation/de/control-cc-schnitte.md \
		Documentation/de/control-cs2-tcp.md \
		Documentation/de/control-cs2-udp.md \
		Documentation/de/control-hsi88.md \
		Documentation/de/control-opendcc-z1.md \
		Documentation/de/control-z21.md \
		Documentation/de/config-rs232-usb.md \
		Documentation/de/config-locomotives.md \
		Documentation/de/config-multipleunits.md \
		Documentation/de/config-layers.md \
		Documentation/de/config-tracks.md \
		Documentation/de/config-groups.md \
		Documentation/de/config-switches.md \
		Documentation/de/config-signals.md \
		Documentation/de/config-accessories.md \
		Documentation/de/config-routes.md \
		Documentation/de/config-feedbacks.md \
		Documentation/de/config-texts.md \
		Documentation/de/config-counter.md \
		Documentation/de/automatic.md \
		--output=Documentation/de/index.html
	pandoc --toc --toc-depth=2 --standalone --css=../style.css \
		--template=Documentation/template.html \
		--metadata-file=Documentation/en/metadata.yaml \
		Documentation/en/index.md \
		Documentation/en/startup-arguments.md \
		Documentation/en/general-settings.md \
		Documentation/en/config-controls.md \
		Documentation/en/config-intellibox.md \
		Documentation/en/config-mastercontrol2.md \
		Documentation/en/config-mastercontrol.md \
		Documentation/en/config-redbox.md \
		Documentation/en/config-twincenter.md \
		Documentation/en/control-6051.md \
		Documentation/en/control-cc-schnitte.md \
		Documentation/en/control-cs2-tcp.md \
		Documentation/en/control-cs2-udp.md \
		Documentation/en/control-hsi88.md \
		Documentation/en/control-opendcc-z1.md \
		Documentation/en/control-z21.md \
		Documentation/en/config-rs232-usb.md \
		Documentation/en/config-locomotives.md \
		Documentation/en/config-multipleunits.md \
		Documentation/en/config-layers.md \
		Documentation/en/config-tracks.md \
		Documentation/en/config-groups.md \
		Documentation/en/config-switches.md \
		Documentation/en/config-signals.md \
		Documentation/en/config-accessories.md \
		Documentation/en/config-routes.md \
		Documentation/en/config-feedbacks.md \
		Documentation/en/config-texts.md \
		Documentation/en/config-counter.md \
		Documentation/en/automatic.md \
		--output=Documentation/en/index.html
	pandoc --toc --toc-depth=2 --standalone --css=../style.css \
		--template=Documentation/template.html \
		--metadata-file=Documentation/es/metadata.yaml \
		Documentation/es/index.md \
		Documentation/es/startup-arguments.md \
		Documentation/es/general-settings.md \
		Documentation/es/config-controls.md \
		Documentation/es/config-intellibox.md \
		Documentation/es/config-mastercontrol2.md \
		Documentation/es/config-mastercontrol.md \
		Documentation/es/config-redbox.md \
		Documentation/es/config-twincenter.md \
		Documentation/es/control-6051.md \
		Documentation/es/control-cc-schnitte.md \
		Documentation/es/control-cs2-tcp.md \
		Documentation/es/control-cs2-udp.md \
		Documentation/es/control-hsi88.md \
		Documentation/es/control-opendcc-z1.md \
		Documentation/es/control-z21.md \
		Documentation/es/config-rs232-usb.md \
		Documentation/es/config-locomotives.md \
		Documentation/es/config-multipleunits.md \
		Documentation/es/config-layers.md \
		Documentation/es/config-tracks.md \
		Documentation/es/config-groups.md \
		Documentation/es/config-switches.md \
		Documentation/es/config-signals.md \
		Documentation/es/config-accessories.md \
		Documentation/es/config-routes.md \
		Documentation/es/config-feedbacks.md \
		Documentation/es/config-texts.md \
		Documentation/es/config-counter.md \
		Documentation/es/automatic.md \
		--output=Documentation/es/index.html

doc-for-web:
	echo 'Nothing to do: The directory structure in Documentation/$LANGUAGE/documentation-$LANGUAGE already is in place.'

