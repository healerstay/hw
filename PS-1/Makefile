all: do-command

.PHONY: all clean run

run: do-command
	./do-command ls -al

do-command.o: do-command.cpp
	g++ do-command.cpp -c -o do-command.o

do-command: do-command.o
	g++ do-command.o -o do-command

clean:
	rm -f *.o do-command
