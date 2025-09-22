#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>
#include <iostream>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " source_file destination_file" << std::endl;
        exit(EXIT_FAILURE);
    }

    const char *src = argv[1];
    const char *dst = argv[2];

    int fd_src = open(src, O_RDONLY);
    if (fd_src == -1) {
        perror("Ошибка при открытии исходного файла");
        exit(EXIT_FAILURE);
    }

    int fd_dst = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_dst == -1) {
        perror("Ошибка при открытии/создании файла назначения");
        close(fd_src);
        exit(EXIT_FAILURE);
    }

    char buffer[4096];
    ssize_t bytes_read;

    while ((bytes_read = read(fd_src, buffer, sizeof(buffer))) > 0) {
        if (write(fd_dst, buffer, bytes_read) == -1) {
            perror("Ошибка при записи в файл назначения");
            close(fd_src);
            close(fd_dst);
            exit(EXIT_FAILURE);
        }
    }

    if (bytes_read == -1) {
        perror("Ошибка при чтении файла источника");
        close(fd_src);
        close(fd_dst);
        exit(EXIT_FAILURE);
    }

    close(fd_src);
    close(fd_dst);
    return 0;
}
