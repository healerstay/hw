#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file-to-write>" << std::endl;
        return 1;
    }

    const char* filename = argv[1];

    int fd1 = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd1 == -1) {
        std::cerr << "Error: open failed: " << std::strerror(errno) << std::endl;
        return 1;
    }

    int fd2 = dup(fd1);
    if (fd2 == -1) {
        std::cerr << "Error: dup failed: " << std::strerror(errno) << std::endl;
        close(fd1);
        return 1;
    }

    const char* line1 = "first line\n";
    const char* line2 = "second line\n";

    ssize_t written = write(fd1, line1, std::strlen(line1));
    if (written != (ssize_t)std::strlen(line1)) {
        std::cerr << "Error: write failed for first line: " << std::strerror(errno) << std::endl;
        close(fd1);
        close(fd2);
        return 1;
    }

    written = write(fd2, line2, std::strlen(line2));
    if (written != (ssize_t)std::strlen(line2)) {
        std::cerr << "Error: write failed for second line: " << std::strerror(errno) << std::endl;
        close(fd1);
        close(fd2);
        return 1;
    }

    close(fd1);
    close(fd2);

    return 0;
}
