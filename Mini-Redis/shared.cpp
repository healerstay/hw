#include "shared.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include <string>
#include <iostream>
#include <atomic>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <algorithm>
#include <unordered_set>

SharedDB* db = nullptr;        
sem_t* mutex = nullptr;                  
sem_t* write_lock = nullptr;             
int* read_count = nullptr;               

int aof_fd = -1;                    
std::atomic<bool> running(true); 

void init_shared_memory() { 
    int shm_fd = shm_open("/mini_redis_shm", O_CREAT | O_RDWR, 0666); 
    if (shm_fd == -1) {
        std::cerr << "failed to shm\n";
        exit(1);
    }

    if (ftruncate(shm_fd, sizeof(SharedDB) + sizeof(int)) == -1) {
        std::cerr << "failed to ftruncate\n";
        close(shm_fd);
        exit(1);
    }

    void* ptr = mmap(0, sizeof(SharedDB) + sizeof(int),
                     PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0); 
    if (ptr == MAP_FAILED) {
        std::cerr << "failed to mmap\n";
        close(shm_fd);
        exit(1);
    }

    db = (SharedDB*)ptr; 
    read_count = (int*)((char*)ptr + sizeof(SharedDB)); 
    *read_count = 0; 

    close(shm_fd);
}

void init_semaphores() { 
    mutex = sem_open("/mini_redis_mutex", O_CREAT, 0666, 1); 
    if (mutex == SEM_FAILED) {
        std::cerr << "failed to mutex\n";
        exit(1);
    }

    write_lock = sem_open("/mini_redis_write", O_CREAT, 0666, 1); 
    if (write_lock == SEM_FAILED) {
        std::cerr << "failed to write_lock\n";
        sem_close(mutex);
        exit(1);
    }
}

void reader_lock() { 
    sem_wait(mutex);
    (*read_count)++;
    if (*read_count == 1) sem_wait(write_lock);
    sem_post(mutex);
}

void reader_unlock() { 
    sem_wait(mutex);
    (*read_count)--;
    if (*read_count == 0) sem_post(write_lock);
    sem_post(mutex);
}

void writer_lock_func() { sem_wait(write_lock); } 
void writer_unlock_func() { sem_post(write_lock); } 

void init_aof() { 
    aof_fd = open("aof", O_CREAT | O_RDWR | O_APPEND, 0666); 
    if (aof_fd == -1) {
        std::cerr << "failed to aof\n";
        exit(1);
    }
}

void append_to_aof(const std::string& cmd) { 
    write(aof_fd, cmd.c_str(), cmd.size()); 
    write(aof_fd, "\n", 1); 
}

void aof_flush_thread() { 
    while (running) {     
        sleep(4);         
        fsync(aof_fd);    
    }
}

void expiration_thread() {
    while (running) {
        sleep(1);

        writer_lock_func();

        time_t now = time(nullptr);

        for (int i = 0; i < MAX_ENTRIES; i++) {
            if (db->entries[i].used && db->entries[i].expire_at != 0 &&
                now >= db->entries[i].expire_at) {
                db->entries[i].used = false;
            }
        }
        
        writer_unlock_func();
    }
}

void load_aof() { 
    std::ifstream file("aof"); 
    if (!file.is_open()) {
        std::cerr << "failed to open aof\n";
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        size_t first = line.find(' ');
        if (first == std::string::npos) continue;

        std::string cmd = line.substr(0, first);
        std::string rest = line.substr(first + 1);

        if (cmd == "SET") {
            size_t second = rest.find(' ');
            if (second == std::string::npos) continue;

            std::string key = rest.substr(0, second);
            std::string value = rest.substr(second + 1);

            for (int i = 0; i < MAX_ENTRIES; i++) {
                if (!db->entries[i].used) {
                    strncpy(db->entries[i].key, key.c_str(), KEY_SIZE);
                    strncpy(db->entries[i].value, value.c_str(), VALUE_SIZE);
                    db->entries[i].used = true;
                    db->entries[i].expire_at = 0;
                    break;
                } else {
                    if (strcmp(db->entries[i].key, key.c_str()) == 0) {
                        strncpy(db->entries[i].key, key.c_str(), KEY_SIZE);
                        strncpy(db->entries[i].value, value.c_str(), VALUE_SIZE);
                        db->entries[i].used = true;
                        db->entries[i].expire_at = 0;
                        break;
                    }
                }
            }
        }
    }
}

void aof_compact_thread() {
    while (running) {
        sleep(15);

        int fd = open("aof", O_RDWR);
        if (fd == -1) continue;

        struct stat st;
        if (fstat(fd, &st) == -1) {
            close(fd);
            continue;
        }

        size_t size = st.st_size;
        if (size == 0) {
            close(fd);
            continue;
        }

        char* buffer = new char[size + 1];
        ssize_t r = read(fd, buffer, size);
        if (r <= 0) {
            delete[] buffer;
            close(fd);
            continue;
        }
        buffer[r] = '\0';

        std::unordered_set<std::string> seen;
        std::vector<std::string> result_lines;

        ssize_t i = r - 1;

        while (i >= 0) {
            ssize_t end = i;

            while (i >= 0 && buffer[i] != '\n') i--;
            ssize_t start = i + 1;

            if (end < start) {
                i--;
                continue;
            }

            std::string line(buffer + start, end - start + 1);

            if (line.compare(0, 4, "SET ") == 0) {
                size_t p1 = line.find(' ', 4);
                if (p1 == std::string::npos) {
                    i--;
                    continue;
                }

                std::string key = line.substr(4, p1 - 4);

                if (seen.insert(key).second) {  // true if new key
                    result_lines.push_back(line + "\n");
                }
            } else if (line.compare(0, 4, "DEL ") == 0) {
                std::string key = line.substr(4);
                seen.insert(key);
            }

            i--;
        }

        delete[] buffer;

        std::string compacted;
        for (int j = result_lines.size() - 1; j >= 0; --j) {
            compacted += result_lines[j];
        }

        writer_lock_func();

        lseek(fd, 0, SEEK_SET);
        size_t written = 0;
        while (written < compacted.size()) {
            ssize_t w = write(fd, compacted.c_str() + written, compacted.size() - written);
            if (w <= 0) break;
            written += w;
        }
        ftruncate(fd, (off_t)written);

        writer_unlock_func();

        close(fd);
    }
}  

void cleanup() { 
    running = false; 

    if (aof_fd != -1) close(aof_fd);

    if (mutex != nullptr) {
        sem_close(mutex);
        sem_unlink("/mini_redis_mutex");
    }

    if (write_lock != nullptr) {
        sem_close(write_lock);
        sem_unlink("/mini_redis_write");
    }

    if (db != nullptr) {
        munmap(db, sizeof(SharedDB) + sizeof(int));
        shm_unlink("/mini_redis_shm");
    }
}
