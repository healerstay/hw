#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>
#include <cstdio>

#define R (50 * 1024 * 1024)

int main() {
    const char* input = "input.txt";
    const char* output = "sorted.txt";

    int fd = open(input, O_RDONLY);
    if (fd== -1) {
        std::cerr << "Cannot open input file\n";
        return 1;
    }
    
    //sort
    std::vector<std::string> lines;
    std::vector<std::string> temp_fs;
    std::vector<char> buffer(R);
    int part = 0;
    int bytes;

    while ((bytes = read(fd, buffer.data(), R)) > 0) {
        size_t start = 0;
        lines.clear();

        std::string partial;
        if (bytes == R) {
            off_t back = 0;
            for (int i = bytes - 1; i >= 0; --i) {
                if (buffer[i] == '\n') {
                    back = bytes - 1 - i;;
                    break;
                }
            }
            if (back > 0) {
                lseek(fd, -back, SEEK_CUR);
                bytes -= back;
            }
        }

        for (int i = 0; i < bytes; ++i) {
            if (buffer[i] == '\n') {
                lines.push_back(partial);
                partial.clear();
            } 
            else {
                partial += buffer[i];
            }
        }

        if (!partial.empty()) { lseek(fd, -partial.size(), SEEK_CUR); }

        std::sort(lines.begin(), lines.end());

        std::string name = "part_" + std::to_string(part++) + ".txt";
        int tmp = open(&name[0], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        for (auto& line : lines) {
            write(tmp, &line[0], line.size());
            write(tmp, "\n", 1);
        }
        close(tmp);
        temp_fs.push_back(name);
    }
    close(fd);

    // merge
    std::vector<int> fds(temp_fs.size());
    std::vector<std::string> curr(temp_fs.size());
    std::vector<bool> unread(temp_fs.size(), true);

    for (int i = 0; i < temp_fs.size(); ++i) {
        fds[i] = open(temp_fs[i].c_str(), O_RDONLY);
        if (fds[i]== -1) {
            std::cerr << "Cannot open " << temp_fs[i] << "\n";
            return 1;
        }

        char c;
        curr[i].clear();
        while ((read(fds[i], &c, 1)) > 0) {
            if (c == '\n') { break; }
            curr[i] += c;
        }
        unread[i] = !curr[i].empty();
    }

    int fd_out = open(output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_out == -1) {
        std::cerr << "Cannot create output file\n";
        return 1;
    }

    while (true) {
        int min_ind = -1;
        for (int i = 0; i < curr.size(); ++i) {
            if (unread[i] == false) { continue; }
            if (min_ind == -1 || curr[i] < curr[min_ind]) { min_ind = i; }
        }
        if (min_ind == -1) { break; }

        write(fd_out, &curr[min_ind][0], curr[min_ind].size());
        write(fd_out, "\n", 1);

        curr[min_ind].clear();
        char c;
        while (read(fds[min_ind], &c, 1)) {
            if (c == '\n') { break; }
            curr[min_ind] += c;
        }
        unread[min_ind] = !curr[min_ind].empty();
    }

    close(fd_out);
    for (int i = 0; i < fds.size(); ++i) {
        close(fds[i]);
        unlink(temp_fs[i].c_str());
    }

    std::cout << "Result: " << output << "\n";
    return 0;
}
