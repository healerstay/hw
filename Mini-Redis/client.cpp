#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string>
#include <vector>

std::vector<std::string> decode(const std::string& s) {
    std::vector<int> sizes;
    std::vector<std::string> res;

    int i = 0;
    while (s[i] != '#') {
        std::string cur;

        while (s[i] != ',') {
            cur += s[i];
            i++;
        }

        sizes.push_back(std::stoi(cur));
        i++;
    }

    i++;
    for (int sz : sizes) {
        res.push_back(s.substr(i, sz));
        i += sz;
    }

    return res;
}

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(1234);

    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);
    connect(sock, (sockaddr*)&server, sizeof(server));

    while (true) {
        std::string cmd;
        std::getline(std::cin, cmd);
        cmd += "\n";
        send(sock, cmd.c_str(), cmd.size(), 0);

        char buf[4096];
        int n = recv(sock, buf, sizeof(buf), 0);
        if (n <= 0)  break;

        std::string encoded(buf, n);
        std::vector<std::string> msgs = decode(encoded);

        for (const std::string& s : msgs) {
            std::cout << s;
        }
    }

    close(sock);
}