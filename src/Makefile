CFLAGS=-g -Wall -Werror
LDFLAGS=-lssl -lcrypto
CC=gcc

mystery: mystery.o util.o phases.o phase_consts.o 
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

dist:
	rm -f *.o *.gch

clean: dist
	rm -f mystery
