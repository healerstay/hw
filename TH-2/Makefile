demo-application: main.o libparallel_scheduler.so
	g++ -o demo-application main.o -L. -lparallel_scheduler -lpthread -Wl,-rpath,.

main.o: main.cpp
	g++ -c main.cpp

libparallel_scheduler.so: parallel_scheduler.o
	g++ -shared -o libparallel_scheduler.so parallel_scheduler.o -lpthread

parallel_scheduler.o: parallel_scheduler.cpp
	g++ -c -fPIC parallel_scheduler.cpp

clean:
	rm -f *.o *.so demo-application

