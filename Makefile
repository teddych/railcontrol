
CC=g++

CPPFLAGS=-g -O2 -Wall -std=c++11
LDFLAGS=-g

OBJ=railcontrol.o

all: $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) -o railcontrol -lpthread

install:
	#install milter-log /usr/sbin

clean:
	rm -f *.o
	rm -f railcontrol

