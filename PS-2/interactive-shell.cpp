#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <cstdlib>
#include <fcntl.h>
#include <vector>
#include <cerrno>

bool silent_mode = false;

int exec_cmd(std::vector<char*>& args) {
    int fd = -1;

    for (int i = 0; i < args.size(); ++i) {
        if (args[i] == nullptr) break;

        if (strcmp(args[i], ">") == 0 || strcmp(args[i], ">>") == 0) {
            bool append = (strcmp(args[i], ">>") == 0);
            if (args[i + 1] == nullptr) {
                if (!silent_mode)
                    std::cerr << "Error: missing file name after '" << args[i] << "'" << std::endl;
                return 1;
            }
            fd = open(args[i + 1],
                      O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC),
                      0644);
            if (fd == -1) {
                if (!silent_mode)
                    std::cerr << "Error opening file '" << args[i + 1]
                              << "': " << strerror(errno) << std::endl;
                return 1;
            }
            args[i] = nullptr;
            break;
        }
    }

    const char* old_path = getenv("PATH");
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        std::string new_path = std::string(cwd);
        if (old_path) {
            new_path += ":" + std::string(old_path);
        }
        setenv("PATH", new_path.c_str(), 1);
    }

    int pid = fork();
    if (pid < 0) {
        if (!silent_mode)
            std::cerr << "Error: fork failed: " << strerror(errno) << std::endl;
        return 1;
    }
    if (pid == 0) {
        if (fd != -1) {
            if (dup2(fd, 1) == -1) {
                if (!silent_mode)
                    std::cerr << "Error redirecting output: " << strerror(errno) << std::endl;
            }
            close(fd);
        }
        execvp(args[0], args.data());
        if (!silent_mode)
            std::cerr << "Error executing '" << args[0]
                      << "': " << strerror(errno) << std::endl;
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

int main(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--silent") == 0 || strcmp(argv[i], "-s") == 0) {
            silent_mode = true;
        }
    }

    std::string line;

    while (true) {
        if (!silent_mode) std::cout << "shell> " << std::flush;
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

            std::string cmd = line.substr(pos, next - pos);
            while (!cmd.empty() && cmd[0] == ' ') cmd.erase(0, 1);
            while (!cmd.empty() && cmd.back() == ' ') cmd.pop_back();

            if (!cmd.empty()) {
                bool execute = true;
                if (prev_op == "&&" && last_status != 0)  execute = false;
                if (prev_op == "||" && last_status == 0) execute = false;
                if (execute) last_status = single_cmd(cmd);
            }

            if (op == "&&" || op == "||") pos = next + 2;
            else if (op == ";") pos = next + 1;
            else pos = line.size();

            prev_op = op;
        }
    }

    return 0;
}
