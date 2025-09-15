all: main

.PHONY: all clean run

run: main
	./main

main.o: main.cpp complex.h sort.h
	g++ main.cpp -c -o main.o

complex.o: complex.cpp complex.h
	g++ complex.cpp -c -o complex.o

main: main.o complex.o
	g++ main.o complex.o -o main

clean:
	rm -f *.o main
