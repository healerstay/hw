#include <iostream>

int main(int argc, char* argv[]) {
    if (argc == 1) {
        std::cout << "Usage: " << argv[0] << " args..." << std::endl;
        return 1;
    }

    for (int i = argc - 1; i >= 0; --i) {
        std::cout << argv[i] << " ";
    }
    std::cout << std::endl;

    return 0;
}

