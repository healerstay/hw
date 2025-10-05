#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <stdlib.h>
#include <cstring>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <input-file>" << std::endl;
        exit(EXIT_FAILURE);
    }

    const char* filename = argv[1];

    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        std::cout << "Error: cannot open file '" << filename << "': " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    if (dup2(fd, 0) == -1) {
        std::cout << "Error: dup2 failed: " << strerror(errno) << std::endl;
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);

    std::string input;
    std::cin >> input;

    std::string reversed(input.rbegin(), input.rend());
    std::cout << reversed  << std::endl;

    return 0;
}
