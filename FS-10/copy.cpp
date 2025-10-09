#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <cmath>
#include <limits>

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

    ssize_t data = 0;
    ssize_t hole = 0;
    char buffer[BUF_SIZE];

    off_t file_end = lseek(src, 0, SEEK_END);
    if (file_end == -1) {
        std::cerr << "Error: " << strerror(errno) << std::endl;
        close(src);
        close(dst);
        return 1;
    }

    off_t curr = 0;
    while (curr < file_end) {
        off_t data_curr = lseek(src, curr, SEEK_DATA);
        if (data_curr == -1) {
            if (errno == ENXIO) {
                hole += file_end - curr;
                break;
            } 
            else {
                std::cerr << "Error seeking data: " << strerror(errno) << std::endl;
                close(src);
                close(dst);
                return 1;
            }
        }
        if (data_curr >= file_end) {
            hole += file_end - curr;
            break;
        }

        if (data_curr > curr) {
            off_t hole_size = data_curr - curr;
            if (lseek(dst, hole_size, SEEK_CUR) == -1) {
                std::cerr << "Error seeking cur: " << strerror(errno) << std::endl;
                close(src);
                close(dst);
                return 1;
            }
            hole += hole_size;
            curr = data_curr;
        }

        off_t hole_curr = lseek(src, curr, SEEK_HOLE);
        if (hole_curr == -1) {
            if (errno == ENXIO) {
                hole_curr = file_end;
            }     
            else {
                std::cerr << "Error seeking hole: " << strerror(errno) << std::endl;
                close(src);
                close(dst);
                return 1;
            }
        }
        
        off_t to_read = hole_curr - curr;
        do {
            size_t chunk = (to_read > BUF_SIZE) ? BUF_SIZE : to_read;
            
            ssize_t r = read(src, buffer, chunk);
            if (r <= 0) {
                std::cerr << "Error: " << strerror(errno) << std::endl;
                close(src);
                close(dst);
                return 1;
            }
            ssize_t w = write(dst, buffer, r);
            if (w != r) {
                std::cerr << "Error: " << strerror(errno) << std::endl;
                close(src);
                close(dst);
                return 1;
            }

            to_read -= r;
            curr += r;
            data += r;
        } while (to_read > 0);
    }

    if (close(src) == -1) {
        std::cerr << "Close source failed: " << strerror(errno) << std::endl;
    }
    if (close(dst) == -1) {
        std::cerr << "Close destination failed: " << strerror(errno) << std::endl;
    }

    std::cout << "Successfully copied " << data + hole << " bytes (data: " << data << ", hole: " << hole << ")." << std::endl;

    return 0;
}
