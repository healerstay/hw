all: manual-append

.PHONY: all clean

manual-append.o: manual-append.cpp
	g++ manual-append.cpp -c -o manual-append.o

manual-append: manual-append.o
	g++ manual-append.o -o manual-append

clean:
	rm -f *.o manual-append
