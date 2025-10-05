#include <iostream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <stdlib.h>
#include <cstring>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <file-to-erase>" << std::endl;
        exit(EXIT_FAILURE);
    }

    const char *filename = argv[1];

    struct stat st;
    if (stat(filename, &st) == -1) {
        std::cout << "Error: stat('" << filename << "') failed: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    int fd = open(filename, O_WRONLY);
    if (fd == -1) {
        std::cout << "Error: open('" << filename << "') failed: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    off_t filesize = lseek(fd, 0, SEEK_END);
    if (filesize == (off_t)-1) {
        std::cout << "Error: lseek(SEEK_END) failed: " << strerror(errno) << std::endl;
        close(fd);
        exit(EXIT_FAILURE);
    }

    if (lseek(fd, 0, SEEK_SET) == (off_t)-1) {
        std::cout << "Error: lseek(SEEK_SET) failed: " << strerror(errno) << std::endl;
        close(fd);
        exit(EXIT_FAILURE);
    }

    constexpr size_t buf_sz = 64 * 1024;
    char buf[buf_sz] = {};

    off_t remaining = filesize;
    while (remaining > 0) {
        size_t tmp = (remaining > (off_t)buf_sz) ? buf_sz : (size_t)remaining;
        ssize_t written = write(fd, buf, tmp);
        if (written <= 0) {
            std::cout << "Error: write failed: " << strerror(errno) << "\n";
            close(fd);
            exit(EXIT_FAILURE);
        }
        remaining -= written;
    }

    if (fsync(fd) == -1) {
        std::cout << "Warning: fsync failed: " << strerror(errno) << std::endl;
    }

    if (close(fd) == -1) {
        std::cout << "Error: close failed: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    if (unlink(filename) == -1) {
        std::cout << "Error: unlink('" << filename << "') failed: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    return 0;
}
