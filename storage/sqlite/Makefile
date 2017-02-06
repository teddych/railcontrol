
CC=gcc
CPP=g++

CFLAGS=-O2 -Wall -I.
LDFLAGS=-ldl -lncurses -pthread

LIBS=shell.o sqlite.o

sqlite3: $(LIBS)
	$(CC) $(LDFLAGS) \
	-DSQLITE_THREADSAFE=0 \
	-DSQLITE_ENABLE_FTS4 \
	-DSQLITE_ENABLE_FTS5 \
	-DSQLITE_ENABLE_JSON1 \
	-DSQLITE_ENABLE_RTREE \
	-DSQLITE_ENABLE_EXPLAIN_COMMENTS \
	-DHAVE_USLEEP \
	-o $@ $(LIBS)

sqlite.o: sqlite3.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o *.so

