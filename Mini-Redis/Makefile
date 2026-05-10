server: server.o utils.o shared.o
	g++ -o server server.o utils.o shared.o -pthread

client: client.o
	g++ -o client client.o

server.o: server.cpp
	g++ -c server.cpp

client.o: client.cpp
	g++ -c client.cpp

utils.o: utils.cpp
	g++ -c utils.cpp

shared.o: shared.cpp
	g++ -c shared.cpp

run: server
	./server

run-client: client
	./client

clean:
	rm -f *.o server client appendonly.aof