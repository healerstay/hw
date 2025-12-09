#include "shared_array.h"
#include <iostream>
#include <unistd.h>

int main() {
    shared_array arr("myarray", 10);

    while (true) {
        arr.lock();
        std::cout << "[Second] ";
        for (int i = 0; i < arr.get_size(); ++i) {
            arr[i] *= 2; 
            std::cout << arr[i] << " ";
        }
        std::cout << std::endl;
        arr.unlock();

        sleep(1);
    }
}

