all: interactive-shell

.PHONY: all clean run

run: interactive-shell
	./interactive-shell

interactive-shell.o: interactive-shell.cpp
	g++ interactive-shell.cpp -c -o interactive-shell.o

interactive-shell: interactive-shell.o
	g++ interactive-shell.o -o interactive-shell

clean:
	rm -f *.o interactive-shell
