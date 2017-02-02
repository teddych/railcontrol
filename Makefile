
CC=g++

#CPPFLAGS=-g -O2 -Wall
#CPPFLAGS=-g -O0 -Wall -std=c++11
CPPFLAGS=-g -O2 -Wall -std=c++11
LDFLAGS=-g -Wl,--whole-archive hardware/cs2.so -Wl,--no-whole-archive -Wl,--export-dynamic
LIBS=-lpthread -ldl

OBJ= \
	control.o \
	hardware_handler.o \
	hardware_properties.o \
	manager.o \
	railcontrol.o \
	util.o \
	webserver.o

all: $(OBJ)
	make -C hardware
	$(CC) $(LDFLAGS) $(OBJ) -o railcontrol $(LIBS)

install:
	#install milter-log /usr/sbin

clean:
	make -C hardware clean
	rm -f *.o
	rm -f railcontrol

