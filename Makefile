CC=g++
CFLAGS=-c -std=c++17 -O0 -g3 -Wall
LDFLAGS=-lboost_system -lboost_program_options -lpthread
LDINC=-L/usr/lib/x86_64-linux-gnu

.PHONY: all clean dependents

all: bin/udp_server bin/udp_client

bin/udp_server: obj/udp_server.o
	mkdir -p bin
	$(CC) $(LDINC) obj/udp_server.o -o bin/udp_server $(LDFLAGS)
	cp run_server.sh bin/run_server.sh

obj/udp_server.o: udp_server.cpp
	mkdir -p obj
	$(CC) $(CFLAGS) udp_server.cpp -o obj/udp_server.o

bin/udp_client: obj/udp_client.o
	mkdir -p bin
	$(CC) $(LDINC) obj/udp_client.o -o bin/udp_client $(LDFLAGS)
	cp run_clients.sh bin/run_clients.sh

obj/udp_client.o: udp_client.cpp
	mkdir -p obj
	$(CC) $(CFLAGS) udp_client.cpp -o obj/udp_client.o
	
clean:
	rm -rf bin
	rm -rf obj

