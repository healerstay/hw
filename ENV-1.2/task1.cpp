#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc != 4) {
        return 1;
    }

    try {
        int a = std::stoi(argv[1]);
        int b = std::stoi(argv[2]);
        int c = std::stoi(argv[3]);

        int max = a;
        if (b > max)
            max = b;
        if (c > max)
            max = c;

        int min = a;
        if (b < min)
            min = b;
        if (c < min)
            min = c;

        std::cout << "min-" << min << ", max-" << max << std::endl;
    }
    catch (const std::exception& e) {
        std::cout << "Invalid input: all arguments must be int" << std::endl;
        return 1;
    }

    return 0;
}

