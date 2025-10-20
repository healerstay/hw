#include <iostream>
#include <cstdlib>
#include <pthread.h>
#include <chrono>

class Data {
public:
    pthread_t tid;
    int* arr;
    long long start;
    long long end;
    long long result;
};

void* sum(void* arg) {
    Data* td = (Data*)arg;
    long long sum = 0;
    for (long long i = td->start; i < td->end; ++i) {
        sum += td->arr[i];
    }
    td->result = sum;
    return nullptr;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <N> <M>" << std::endl;
        return 1;
    }

    long long N = atoll(argv[1]);
    int M = atoi(argv[2]);
    if (N <= 1000000 || M <= 0) {
        std::cerr << "Error: N must be > 1,000,000 and M > 0" << std::endl;
        return 1;
    }

    int* arr = new int[N];
    for (long long i = 0; i < N; ++i)
        arr[i] = rand() % 1000;

    auto start1 = std::chrono::high_resolution_clock::now();
    long long sum1 = 0;
    for (long long i = 0; i < N; ++i) {
        sum1 += arr[i];
    }
    auto end1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> dur1 = end1 - start1;

    Data* thread_data = new Data[M];
    long long chunk = N / M;

    auto start2 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < M; ++i) {
        thread_data[i].arr = arr;
        thread_data[i].start = i * chunk;
        thread_data[i].end = (i == M - 1) ? N : (i + 1) * chunk;
        pthread_create(&thread_data[i].tid, nullptr, sum, &thread_data[i]);
    }

    long long sum2 = 0;
    for (int i = 0; i < M; ++i) {
        pthread_join(thread_data[i].tid, nullptr);
        sum2 += thread_data[i].result;
    }
    auto end2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> dur2 = end2 - start2;

    std::cout << "Time spent without threads: " << dur1.count() << " seconds" << std::endl;
    std::cout << "Time spent with " << M << " threads: " << dur2.count() << " seconds" << std::endl;

    delete[] arr;
    delete[] thread_data;

    return 0;
}
