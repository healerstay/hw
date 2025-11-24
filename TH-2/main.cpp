#include "parallel_scheduler.h"
#include <iostream>
#include <pthread.h>

pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

void task(int x) {
    pthread_mutex_lock(&print_mutex);
    std::cout << "Task " << x << " running\n";
    pthread_mutex_unlock(&print_mutex);
}


int main() {
    parallel_scheduler ps(3);

    for (int i = 1; i <= 10; ++i) {
        ps.run(task, i);
    }

    return 0;
}

