#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <cerrno>

void do_command(char** argv) {
    time_t start_time = time(nullptr);
    
    pid_t child = fork();
    if (child == -1){
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
    else if (child == 0) {
        execvp(argv[0], argv);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    }
    else {
        int status;
        wait(&status);
        time_t end_time = time(nullptr);

        std::cout << "Command completed with " << WEXITSTATUS(status)  << " exit code and took " 
                  << difftime(end_time, start_time) << " seconds." << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " [args...]" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    do_command(argv + 1);
    
    return 0;
}
