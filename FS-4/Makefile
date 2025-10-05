all: redirect-cin

.PHONY: all clean

redirect-cin.o: redirect-cin.cpp
	g++ redirect-cin.cpp -c -o redirect-cin.o

redirect-cin: redirect-cin.o
	g++ redirect-cin.o -o redirect-cin

clean:
	rm -f *.o redirect-cin
