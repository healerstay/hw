#include "parallel_scheduler.h"

parallel_scheduler::parallel_scheduler(int cap) : stop(false), capacity(cap) {
    pthread_mutex_init(&mutex, nullptr);
    pthread_cond_init(&cond, nullptr);

    threads = new pthread_t[capacity];

    for (int i = 0; i < capacity; ++i) {
        pthread_create(&threads[i], nullptr, worker, this);
    }
}

parallel_scheduler::~parallel_scheduler() {
    pthread_mutex_lock(&mutex);
    stop = true;
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);

    for (int i = 0; i < capacity; ++i) {
        pthread_join(threads[i], nullptr);
    }

    delete[] threads;
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
}

void parallel_scheduler::run(void (*func)(int), int arg) {
    pthread_mutex_lock(&mutex);
    tasks.push({func, arg});
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

void* parallel_scheduler::worker(void* arg) {
    auto* pool = (parallel_scheduler*)arg;

    while (true) {
        pthread_mutex_lock(&pool->mutex);

        while (!pool->stop && pool->tasks.empty()) {
            pthread_cond_wait(&pool->cond, &pool->mutex);
        }

        if (pool->stop && pool->tasks.empty()) {
            pthread_mutex_unlock(&pool->mutex);
            return nullptr;
        }

        Task task = pool->tasks.front();
        pool->tasks.pop();

        pthread_mutex_unlock(&pool->mutex);

        task.func(task.arg);
    }
}
