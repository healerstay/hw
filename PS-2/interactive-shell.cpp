#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <cstdlib>
#include <fcntl.h>
#include <vector>

int exec_cmd(std::vector<char*>& args) {
    int fd = -1;

    for (int i = 0; i < args.size(); ++i) {
        if (args[i] == nullptr) break;

        if (strcmp(args[i], ">") == 0) {
            if (args[i + 1] == nullptr) {
                std::cerr << "Error: missing file name after '>'" << std::endl;
                return 1;
            }
            fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd == -1) {
                std::cerr << "Error opening file" << std::endl;
                return 1;
            }
            args[i] = nullptr;
            break;
        }

        if (strcmp(args[i], ">>") == 0) {
            if (args[i + 1] == nullptr) {
                std::cerr << "Error: missing file name after '>>'" << std::endl;
                return 1;
            }
            fd = open(args[i + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (fd == -1) {
                std::cerr << "Error opening file" << std::endl;
                return 1;
            }
            args[i] = nullptr;
            break;
        }
    }

    int pid = fork();
    if (pid < 0) {
        std::cerr << "Error while fork" << std::endl;
        return 1;
    }
    if (pid == 0) {
        if (fd != -1) {
            if (dup2(fd, 1) == -1) {
                std::cerr << "Fail opening file" << std::endl;
            }
            close(fd);
        }
        execvp(args[0], args.data());
        std::cerr << "Error while exec" << std::endl;
        exit(1);
    } 
    else {
        int status;
        wait(&status);
        if (WIFEXITED(status)) { 
            return WEXITSTATUS(status);
        } 
        else { return 1; }
    }
}

int single_cmd(const std::string &cmd) {
    std::vector<char*> args;
    char* cstr = new char[cmd.length() + 1];
    std::strcpy(cstr, cmd.c_str());

    char* token = std::strtok(cstr, " ");
    while (token != nullptr) {
        args.push_back(token);
        token = std::strtok(nullptr, " ");
    }
    args.push_back(nullptr);

    int result = exec_cmd(args);
    delete[] cstr;
    return result;
}

int main() {
    std::string line;

    while (true) {
        std::cout << "shell> ";
        std::getline(std::cin, line);
        if (line == "exit") { break; }
        if (line.empty()) { continue; }

        int last_status = 0;
        std::string prev_op = "";
        int pos = 0;

        while (pos < line.size()) {
            int next = line.size();
            std::string op = "";

            for (int i = pos; i + 1 < line.size(); ++i) {
                if (line[i] == ';') {
                    next = i;
                    op = ";";
                    break;
                }
                if (line[i] == '&' && line[i + 1] == '&') {
                    next = i;
                    op = "&&";
                    break;
                }
                if (line[i] == '|' && line[i + 1] == '|') {
                    next = i;
                    op = "||";
                    break;
                }
            }
            
            //trim
            std::string cmd = line.substr(pos, next - pos);
            while (cmd.empty() == 0 && cmd[0] == ' ') {
                cmd.erase(0, 1);
            }
            while (cmd.empty() == 0 && cmd[cmd.size() - 1] == ' ') {
                cmd.erase(cmd.size() - 1, 1);
            }

            if (cmd.empty() == 0) {
                bool tmp = true;
                if (prev_op == "&&" && last_status != 0)  { tmp = false; }
                if (prev_op == "||" && last_status == 0) { tmp = false; }
                if (tmp) { last_status = single_cmd(cmd); }
            }

            if (op == "&&" || op == "||") { pos = next + 2; }
            else if (op == ";") { pos = next + 1; }
            else { pos = line.size(); }

            prev_op = op;
        }
    }

    return 0;
}
