#ifndef PARALLEL_SCHEDULER_H
#define PARALLEL_SCHEDULER_H

#include <pthread.h>
#include <queue>

class parallel_scheduler {
private:
    struct Task {
        void (*func)(int);
        int arg;
    };

    std::queue<Task> tasks;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    bool stop;

    pthread_t* threads;
    int capacity;

    static void* worker(void* arg);

public:
    explicit parallel_scheduler(int capacity);
    ~parallel_scheduler();

    void run(void (*func)(int), int arg);
};

#endif
