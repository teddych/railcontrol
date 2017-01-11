
CC=gcc

CFLAGS=-g -O2 -Wall
#CFLAGS=-g -O0 -Wall
LDFLAGS=-g

OBJ=railcontrol.o

all: $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) -o railcontrol -lpthread

install:
	#install milter-log /usr/sbin

clean:
	rm -f *.o
	rm -f railcontrol

