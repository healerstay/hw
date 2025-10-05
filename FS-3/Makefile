all: hacker-rm

.PHONY: all clean

hacker-rm.o: hacker-rm.cpp
	g++ hacker-rm.cpp -c -o hacker-rm.o

hacker-rm: hacker-rm.o
	g++ hacker-rm.o -o hacker-rm

clean:
	rm -f *.o hacker-rm
