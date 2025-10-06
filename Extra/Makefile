all: fsort

.PHONY: all run clean

run: fsort
	./fsort input.txt output.txt

fsort.o: fsort.cpp
	g++ fsort.cpp -c -o fsort.o

fsort: fsort.o
	g++ fsort.o -o fsort

clean:
	rm -f *.o fsort
