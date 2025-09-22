all: simple-copy

.PHONY: all run clean

run: simple-copy
	./simple-copy source.txt destination.txt

simple-copy.o: simple-copy.cpp
	g++ simple-copy.cpp -c -o simple-copy.o

simple-copy: simple-copy.o
	g++ simple-copy.o -o simple-copy

clean:
	rm -f *.o simple-copy

