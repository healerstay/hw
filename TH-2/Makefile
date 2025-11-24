all: demo-application

parallel_scheduler.o: parallel_scheduler.cpp
	g++ -fPIC -c parallel_scheduler.cpp -o parallel_scheduler.o

parallel_scheduler.so: parallel_scheduler.o
	g++ -shared -o parallel_scheduler.so parallel_scheduler.o -lpthread

main.o: main.cpp
	g++ -c main.cpp -o main.o

demo-application: main.o parallel_scheduler.so
	g++ main.o ./parallel_scheduler.so -lpthread -o demo-application

clean:
	rm -f *.o *.so demo-application

