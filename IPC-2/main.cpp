#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <string>

int prime(int m) {
    if (m <= 0 || m > 100) {
        return -1;
    }
    int res = 2, count  = 1;
    bool isPrime;
    while(true) {
        isPrime = true;
        for (int i = 2; i * i <= res; i++) {
            if (res % i == 0) {
                isPrime = false;
                break;
            }
        }
        if (count == m && isPrime) {
            return res;
        }
        if (isPrime) {
            ++count;
        }
        ++res;
    }
}

int main()
{
    int toParent[2];
    int toChild[2];
    if (pipe(toParent) == -1 || pipe(toChild) == -1) {
        perror("failed pipe");
        return 1;
    }
    
    pid_t pid = fork();
    if (pid > 0) {
        close(toChild[0]);
        close(toParent[1]); 
        
        while(true) {
            std::string input;
            std::cout << "[Parent] Please enter the number: ";
            std::cin >> input;

            if (input == "exit") {
                close(toChild[1]);
                break;
            }
            
            int m = std::stoi(input);
            if (m <= 0 || m > 100) {
                std::cout << "[Parent] Input must be > 0 and <= 100" << std::endl;
                continue;
            }

            std::cout << "[Parent] Sending " << m << " to the child process..." << std::endl;
            if (write(toChild[1], &m, sizeof(m)) < 0) {
                perror("failed write to child");
                return 1;
            }

            int res;
            std::cout << "[Parent] Waiting for the response from the child process..." << std::endl;
            if (read(toParent[0], &res, sizeof(res)) < 0) {
                perror("failed read to parent");
                return 1;
            }
            
            std::cout << "Received calculation result of prime(" << m << ") = " << res << "..." << std::endl;
            
        }
        
        close(toChild[1]);
        close(toParent[0]);
        wait(nullptr);
        
    }
    else if (pid == 0) {
        close(toChild[1]); 
        close(toParent[0]);
        
        while (true) {
            int m;
            
            int r = read(toChild[0], &m, sizeof(m));
            if (r < 0) {
                perror("failed read to child");
                return 1;
            }
            else if (r == 0) {
                break;
            }

            std::cout << "[Child] Calculating " << m << "-th prime number..." << std::endl;
            int res = prime(m);
            std::cout << "[Child] Sending calculation result of prime(" << m << ")..." << std::endl;
            if (write(toParent[1], &res, sizeof(res)) < 0) {
                perror("failed write to parent");
                return 1;
            }
        }
        
        close(toChild[0]); 
        close(toParent[1]);
    }
    else {
        perror("failed fork");
        return 1;
    }

    return 0;
}
