all: first second

first: first.cpp shared_array.h
	g++ -pthread -o first first.cpp

second: second.cpp shared_array.h
	g++ -pthread -o second second.cpp

clean:
	rm -f *.o first second

