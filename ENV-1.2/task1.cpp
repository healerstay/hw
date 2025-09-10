#include <iostream>
#include <string>

bool check(const std::string& s, int& value) {
    try {
        size_t pos;
        value = std::stoi(s, &pos);
        return pos == s.size();
    } 
    catch (...) {
        return false;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " int1 int2 int3" << std::endl;
        return 1;
    }

    int a, b, c;
    if (!check(argv[1], a) || !check(argv[2], b) || !check(argv[3], c)) {
        std::cout << "Invalid input: all arguments must be integers" << std::endl;
        return 1;
    }

    int max = a;
    if (b > max) max = b;
    if (c > max) max = c;

    int min = a;
    if (b < min) min = b;
    if (c < min) min = c;

    std::cout << "min-" << min << ", max-" << max << std::endl;
    return 0;
}

