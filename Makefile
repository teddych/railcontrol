
CC=g++

CPPFLAGS=-g -O2 -Wall
#CPPFLAGS=-g -O2 -Wall -std=c++11
LDFLAGS=-g
LIBS=-lpthread -ldl

OBJ=util.o railcontrol.o

all: $(OBJ)
	make -C hardware
	$(CC) $(LDFLAGS) $(OBJ) -o railcontrol $(LIBS)

install:
	#install milter-log /usr/sbin

clean:
	make -C hardware clean
	rm -f *.o
	rm -f railcontrol

