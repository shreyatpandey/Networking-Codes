CC=g++
CFLAGS=-c -Wall


all: client server

server: server.o
		$(CC) -o server -g -pthread server.o

server.o: server.cpp header_server.h
		$(CC) -std=c++11 -g $(CFLAGS) server.cpp 

client: client.o
		$(CC) -o client -g -pthread client.o

client.o: client.cpp header_client.h
		$(CC) -std=c++11 -g $(CFLAGS) client.cpp  

clean:
		rm -f *.o all
		
