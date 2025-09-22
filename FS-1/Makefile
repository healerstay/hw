all: read-file

.PHONY: all clean run

run: read-file
	./read-file input.txt

read-file.o: read-file.cpp
	g++ read-file.cpp -c -o read-file.o

read-file: read-file.o
	g++ read-file.o -o read-file

clean:
	rm -f *.o read-file
