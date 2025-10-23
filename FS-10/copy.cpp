#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>

constexpr size_t BUF_SIZE = 64 * 1024;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <source-file> <destination-file>" << std::endl;
        return 1;
    }

    const char* src_path = argv[1];
    const char* dst_path = argv[2];

    int fd_src = open(src_path, O_RDONLY);
    if (fd_src == -1) {
        std::cerr << "Error: cannot open source file '" << src_path << "': " 
                  << strerror(errno) << std::endl;
        return 1;
    }

    struct stat st;
    if (fstat(fd_src, &st) == -1) {
        std::cerr << "Error: fstat failed: " << strerror(errno) << std::endl;
        close(fd_src);
        return 1;
    }

    off_t file_size = st.st_size;

    int fd_dst = open(dst_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_dst == -1) {
        std::cerr << "Error: cannot open destination file '" << dst_path << "': " << strerror(errno) << std::endl;
        close(fd_src);
        return 1;
    }

    char buffer[BUF_SIZE];
    off_t data_bytes = 0;
    off_t hole_bytes = 0;
    off_t offset = 0;

    bool seek_supported = true;
    if (lseek(fd_src, 0, SEEK_DATA) == -1 && errno == ENXIO) {
        seek_supported = false;
    }
    lseek(fd_src, 0, SEEK_SET);

    if (!seek_supported) {
        ssize_t total = 0;
        while (true) {
            ssize_t r = read(fd_src, buffer, BUF_SIZE);
            if (r < 0) {
                if (errno == ENXIO) { continue; }
                std::cerr << "Read error: " << strerror(errno) << std::endl;
                close(fd_src);
                close(fd_dst);
                return 1;
            }
            if (r == 0) { break; }

            ssize_t written = 0;
            while (written < r) {
                ssize_t w = write(fd_dst, buffer + written, r - written);
                if (w < 0) {
                    if (errno == ENXIO) { continue; }
                    std::cerr << "Write error: " << strerror(errno) << std::endl;
                    close(fd_src);
                    close(fd_dst);
                    return 1;
                }
                written += w;
            }
            total += r;
        }
        std::cout << "Successfully copied " << total 
                  << " bytes (data: " << total << ", hole: 0)" << std::endl;
        close(fd_src);
        close(fd_dst);
        return 0;
    }

    while (offset < file_size) {
        off_t data_off = lseek(fd_src, offset, SEEK_DATA);
        if (data_off == -1) {
            if (errno == ENXIO) { break; }
            std::cerr << "SEEK_DATA error: " << strerror(errno) << std::endl;
            close(fd_src);
            close(fd_dst);
            return 1;
        }

        if (data_off > offset) {
            off_t hole_len = data_off - offset;
            if (lseek(fd_dst, hole_len, SEEK_CUR) == -1) {
                std::cerr << "Destination lseek error: " << strerror(errno) << std::endl;
                close(fd_src);
                close(fd_dst);
                return 1;
            }
            hole_bytes += hole_len;
        }

        off_t hole_off = lseek(fd_src, data_off, SEEK_HOLE);
        if (hole_off == -1) {
            std::cerr << "SEEK_HOLE error: " << strerror(errno) << std::endl;
            close(fd_src);
            close(fd_dst);
            return 1;
        }

        off_t to_copy = hole_off - data_off;
        if (lseek(fd_src, data_off, SEEK_SET) == -1) {
            std::cerr << "Source lseek error: " << strerror(errno) << std::endl;
            close(fd_src);
            close(fd_dst);
            return 1;
        }

        while (to_copy > 0) {
            size_t chunk = (to_copy > BUF_SIZE) ? BUF_SIZE : to_copy;
            ssize_t r = read(fd_src, buffer, chunk);
            if (r <= 0) {
                std::cerr << "Read error: " << strerror(errno) << std::endl;
                close(fd_src);
                close(fd_dst);
                return 1;
            }

            ssize_t written = 0;
            while (written < r) {
                ssize_t w = write(fd_dst, buffer + written, r - written);
                if (w < 0) {
                    if (errno == ENXIO) { continue; }
                    std::cerr << "Write error: " << strerror(errno) << std::endl;
                    close(fd_src);
                    close(fd_dst);
                    return 1;
                }
                written += w;
                data_bytes += w;
            }
            to_copy -= r;
        }
        offset = hole_off;
    }

    if (file_size > 0 && ftruncate(fd_dst, file_size) == -1) {
        std::cerr << "Warning: ftruncate failed: " << strerror(errno) << std::endl;
    }

    if (close(fd_src) == -1) {
        std::cerr << "Close source failed: " << strerror(errno) << std::endl;
    }
    if (close(fd_dst) == -1) {
        std::cerr << "Close destination failed: " << strerror(errno) << std::endl;
    }

    std::cout << "Successfully copied " << file_size << " bytes (data: " << data_bytes << ", hole: " << hole_bytes << ")." << std::endl;

    return 0;
}
