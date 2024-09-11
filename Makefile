CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -ljson-c

all: server shutdown_server

server: server.o file_path.o
	$(CC) $(CFLAGS) -o server server.o file_path.o $(LDFLAGS)

shutdown_server: shutdown_server.o file_path.o
	$(CC) $(CFLAGS) -o shutdown_server shutdown_server.o file_path.o $(LDFLAGS)

server.o: server.c file_path.h
	$(CC) $(CFLAGS) -c server.c

file_path.o: file_path.c file_path.h
	$(CC) $(CFLAGS) -c file_path.c

shutdown_server.o: shutdown_server.c file_path.h
	$(CC) $(CFLAGS) -c shutdown_server.c

clean:
	rm -f *.o server shutdown_server
