
CC=g++

#CPPFLAGS=-g -O2 -Wall
#CPPFLAGS=-g -O0 -Wall -std=c++11
CPPFLAGS=-I. -g -O2 -Wall -std=c++11
#LDFLAGS=-g -Wl,--whole-archive -Wl,--no-whole-archive -Wl,--export-dynamic
LDFLAGS=-g -Wl,--export-dynamic
LIBS=-lpthread -ldl

OBJ= \
	control.o \
	datamodel/loco.o \
	hardware/hardware_handler.o \
	manager.o \
	railcontrol.o \
	storage/storage_handler.o \
	util.o \
	webserver/webserver.o \
	webserver/webclient.o

all: $(OBJ)
	make -C hardware
	make -C storage
	make -C storage/sqlite
	$(CC) $(LDFLAGS) $(OBJ) -o railcontrol $(LIBS)

%.o: %.cpp %.h
	$(CC) $(CPPFLAGS) -c -o $@ $<

install:
	#install milter-log /usr/sbin

clean:
	make -C hardware clean
	make -C storage clean
	make -C storage/sqlite clean
	rm -f *.o webserver/*.o
	rm -f railcontrol

