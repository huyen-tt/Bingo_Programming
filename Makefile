CC=gcc
CFLAGS=-O3 -Wall
LIBS=-pthread
SRC=src

all: server client
server: server.o
	$(CC) $(CFLAGS) $(LIBS) -o server server.o
client: client.o string.o
	$(CC) $(CFLAGS) $(LIBS) -o client client.o string.o
server.o: $(SRC)/server.c
	$(CC) $(CFLAGS) -c $(SRC)/server.c
string.o: $(SRC)/string.c
	$(CC) $(CFLAGS) -c $(SRC)/string.c
client.o: $(SRC)/client.c
	$(CC) $(CFLAGS) -c $(SRC)/client.c

.PHONY: clean
clean:
	rm -f *.o *.out
