#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>

constexpr size_t BUF_SIZE = 64 * 1024;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <source-file> <destination-file>" << std::endl;
        return 1;
    }

    const char* src_path = argv[1];
    const char* dst_path = argv[2];

    int src = open(src_path, O_RDONLY);
    if (src == -1) {
        std::cerr << "Error: cannot open source file '" << src_path << "': " << strerror(errno) << std::endl;
        return 1;
    }

    int dst = open(dst_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dst == -1) {
        std::cerr << "Error: cannot open destination file '" << dst_path << "': " << strerror(errno) << std::endl;
        close(src);
        return 1;
    }

    char buffer[BUF_SIZE];
    ssize_t data = 0;
    ssize_t hole = 0;

    off_t file_end = lseek(src, 0, SEEK_END);
    if (file_end == -1) {
        std::cerr << "Error: " << strerror(errno) << std::endl;
        close(src);
        close(dst);
        return 1;
    }

    off_t curr = 0;
    while (curr < file_end) {
        off_t data_off = lseek(src, curr, SEEK_DATA);
        off_t hole_off;

        if (data_off == -1) {
            if (errno == ENXIO) {
                off_t hole_size = file_end - curr;
                if (hole_size > 0) {
                    if (lseek(dst, hole_size, SEEK_CUR) == -1) {
                        std::cerr << "Error seeking dst: " << strerror(errno) << std::endl;
                        close(src);
                        close(dst);
                        return 1;
                    }
                    hole += hole_size;
                }
                break;
            } else {
                std::cerr << "Error seeking data: " << strerror(errno) << std::endl;
                close(src);
                close(dst);
                return 1;
            }
        }

        if (data_off > curr) {
            off_t hole_size = data_off - curr;
            if (lseek(dst, hole_size, SEEK_CUR) == -1) {
                std::cerr << "Error seeking dst (hole): " << strerror(errno) << std::endl;
                close(src);
                close(dst);
                return 1;
            }
            hole += hole_size;
            curr = data_off;
        }

        hole_off = lseek(src, curr, SEEK_HOLE);
        if (hole_off == -1) {
            if (errno == ENXIO)
                hole_off = file_end;
            else {
                std::cerr << "Error seeking hole: " << strerror(errno) << std::endl;
                close(src);
                close(dst);
                return 1;
            }
        }

        off_t to_copy = hole_off - curr;
        while (to_copy > 0) {
            size_t chunk = (to_copy > BUF_SIZE) ? BUF_SIZE : to_copy;
            ssize_t r = read(src, buffer, chunk);
            if (r <= 0) {
                std::cerr << "Read error: " << strerror(errno) << std::endl;
                close(src);
                close(dst);
                return 1;
            }

            ssize_t w = write(dst, buffer, r);
            if (w != r) {
                std::cerr << "Write error: " << strerror(errno) << std::endl;
                close(src);
                close(dst);
                return 1;
            }

            data += r;
            curr += r;
            to_copy -= r;
        }
    }

    if (ftruncate(dst, file_end) == -1) {
        std::cerr << "Warning: ftruncate failed: " << strerror(errno) << std::endl;
    }

    if (close(src) == -1)
        std::cerr << "Close source failed: " << strerror(errno) << std::endl;
    if (close(dst) == -1)
        std::cerr << "Close destination failed: " << strerror(errno) << std::endl;

    std::cout << "Successfully copied " << data + hole
              << " bytes (data: " << data << ", hole: " << hole << ")."
              << std::endl;

    return 0;
}
