all: copy

.PHONY: all clean

copy.o: copy.cpp
	g++ copy.cpp -c -o copy.o

copy: copy.o
	g++ copy.o -o copy

clean:
	rm -f *.o copy
