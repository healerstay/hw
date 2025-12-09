#ifndef SHARED_ARRAY_H
#define SHARED_ARRAY_H

#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <cstdlib>
#include <unistd.h>
#include <semaphore.h>

class shared_array {
    std::string name;
    size_t size;
    int* data;
    int fd;
    sem_t* sem;

public:
    shared_array(const std::string& n, size_t s) : name(n), size(s), data(nullptr), fd(-1), sem(nullptr) {
        fd = shm_open(("/" + name).c_str(), O_CREAT | O_EXCL | O_RDWR, 0644);
	if (fd >= 0) {
    		if (ftruncate(fd, sizeof(int) * size) < 0) {
        		perror("ftruncate failed");
        		exit(EXIT_FAILURE);
    		}	
	}
	 else if (errno == EEXIST) {
    		//if was already exists
    		fd = shm_open(("/" + name).c_str(), O_RDWR, 0644);
    		if (fd < 0) {
        		perror("shm_open failed");
        		exit(EXIT_FAILURE);
    		}
	} 
	else {
    		perror("shm_open failed");
    		exit(EXIT_FAILURE);
	}	


        data = (int*)mmap(nullptr, sizeof(int) * size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (data == MAP_FAILED) {
            perror("mmap failed");
            exit(EXIT_FAILURE);
        }

        sem = sem_open(("/" + name + "_sem").c_str(), O_CREAT, 0644, 1);
        if (sem == SEM_FAILED) {
            perror("sem_open failed");
            exit(EXIT_FAILURE);
        }
    }

    ~shared_array() {
        munmap(data, sizeof(int) * size);
        close(fd);
        sem_close(sem);
    }

    int& operator[](size_t index) {
        return data[index];
    }

    void lock() { sem_wait(sem); }
    void unlock() { sem_post(sem); }
    size_t get_size() const { return size; }
};

#endif
