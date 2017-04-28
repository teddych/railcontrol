
CC=g++

#CPPFLAGS=-g -O2 -Wall
#CPPFLAGS=-g -O0 -Wall -std=c++11
CPPFLAGS=-I. -g -O0 -Wall -std=c++11
#CPPFLAGS=-I. -g -O2 -Wall -std=c++11
LDFLAGS=-g -Wl,--export-dynamic
LIBS=-lpthread -ldl

OBJ= \
	config.o \
	datamodel/accessory.o \
	datamodel/block.o \
	datamodel/feedback.o \
	datamodel/layout_item.o \
	datamodel/loco.o \
	datamodel/relation.o \
	datamodel/serializable.o \
	datamodel/street.o \
	datamodel/switch.o \
	hardware/hardware_handler.o \
	hardware/hardware_params.o \
	manager.o \
	railcontrol.o \
	storage/storage_handler.o \
	util.o \
	webserver/webserver.o \
	webserver/webclient.o

all: $(OBJ)
	make -C hardware
	make -C storage
	$(CC) $(LDFLAGS) $(OBJ) -o railcontrol $(LIBS)

sqlite-shell:
	make -C storage/sqlite

%.o: %.cpp *.h datamodel/*.h webserver/*.h storage/*.h hardware/*.h
	$(CC) $(CPPFLAGS) -c -o $@ $<

install:
	#install milter-log /usr/sbin

clean:
	make -C hardware clean
	make -C storage clean
	rm -f *.o webserver/*.o datamodel/*.o
	rm -f railcontrol

clean-sqlite-shell:
	make -C storage/sqlite clean

test:
	make -C test
