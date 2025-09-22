#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>
#include <iostream>

int main(int argc, char *argv[]) {
	if (argc < 2) {
		std::cout << "Usage: " << argv[0] << " file_path" << std::endl;
		exit(EXIT_FAILURE);
	}
	const char *filepath = argv[1];
	int fd = open(filepath, O_RDONLY);
	if (fd == -1) {
		perror("Ошибка при открытии файла");
		exit(EXIT_FAILURE);
	}

	char buffer[4096];
	ssize_t bytes_read;

	while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
		if (write(STDOUT_FILENO, buffer, bytes_read) == -1) {
			perror("Ошибка при выводе в stdout");
			close(fd);
			exit(EXIT_FAILURE);
		}
	}

	if (bytes_read == -1) {
		perror("Ошибка при чтении файла");
		close(fd);
                exit(EXIT_FAILURE);
	}

	close(fd);
	return 0;
}
