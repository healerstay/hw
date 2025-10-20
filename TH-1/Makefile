all: array-summary

.PHONY: all clean run

N ?= 10000000
M ?= 4

run: array-summary
	./array-summary $(N) $(M)

array-summary.o: array-summary.cpp
	g++ array-summary.cpp -c -o array-summary.o -O2 -pthread

array-summary: array-summary.o
	g++ array-summary.o -o array-summary -O2 -pthread

clean:
	rm -f *.o array-summary
