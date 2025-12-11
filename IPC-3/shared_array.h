#ifndef SHARED_ARRAY_H
#define SHARED_ARRAY_H

#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <cstdlib>
#include <unistd.h>
#include <semaphore.h>
#include <cerrno>
#include <sys/stat.h>

class shared_array {
    std::string name;
    size_t size;
    int* data;
    int fd;
    sem_t* sem;
    bool created = false;

public:
    shared_array(const std::string& n, size_t s) {
        if (s > 1000000000) {
            throw std::out_of_range("size should be < 1 000 000 000");
        }
        name = n;
        size = s;
        data = nullptr;
        fd = -1;
        sem = nullptr;
    
        fd = shm_open(("/" + name).c_str(), O_CREAT | O_EXCL | O_RDWR, 0644);
    	if (fd >= 0) {
    	    created = true;
    		if (ftruncate(fd, sizeof(int) * size) < 0) {
        		perror("ftruncate failed");
        		shm_unlink(("/" + name).c_str());
        		close(fd);
        		exit(EXIT_FAILURE);
    		}	
    	}
    	 else if (errno == EEXIST) {
    		//if already exists
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
        
        struct stat sb;
        if (fstat(fd, &sb) < 0) {
            perror("fstat failed");
            exit(EXIT_FAILURE);
        }

        data = (int*)mmap(nullptr, sizeof(int) * size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (data == MAP_FAILED) {
            perror("mmap failed");
            if (created) { shm_unlink(("/" + name).c_str()); }
            close(fd); 
            exit(EXIT_FAILURE);
        }

        sem = sem_open(("/" + name + "_sem").c_str(), O_CREAT, 0644, 1);
        if (sem == SEM_FAILED) {
            perror("sem_open failed"); 
            exit(EXIT_FAILURE);
        }
    }

    ~shared_array() {
        if (data) { munmap(data, sizeof(int) * size); }
        if (fd >= 0) { close(fd); }
        if (sem && sem != SEM_FAILED) { sem_close(sem); }
        if (created) {
            shm_unlink(("/" + name).c_str());
            sem_unlink(("/" + name + "_sem").c_str());
        }
    }
    
    shared_array(const shared_array&) = delete;
    shared_array& operator=(const shared_array&) = delete;
    shared_array(shared_array&& other) : name(std::move(other.name)), size(other.size),
                                        data(other.data), fd(other.fd), sem(other.sem) {
        other.data = nullptr;
        other.fd = -1;
        other.sem = nullptr;
        other.size = 0;
    }

    int& operator[](size_t index) {
        if (index >= size) {
            throw std::out_of_range("index is out of range");
        }
        return data[index];
    }

    void lock() { sem_wait(sem); }
    void unlock() { sem_post(sem); }
    size_t get_size() const { return size; }
};

#endif
