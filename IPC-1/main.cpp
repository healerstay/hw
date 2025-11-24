#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <pwd.h>
#include <sys/ucontext.h>

void handler(int, siginfo_t* info, void* context) {
    std::cout << "Received SIGUSR1 from PID " << info->si_pid
              << ", UID " << info->si_uid << " (" 
              << getpwuid(info->si_uid)->pw_name << ")." << std::endl;

    auto uctx = (ucontext_t*)context;
/*    std::cout << "Registers: EIP=" << uctx->uc_mcontext.gregs[REG_EIP]
              << ", EAX=" << uctx->uc_mcontext.gregs[REG_EAX]
              << ", EBX=" << uctx->uc_mcontext.gregs[REG_EBX] << "." << std::endl;
*/
}

int main() {
    std::cout << "PID: " << getpid() << "\n";

    struct sigaction new_action = {};
    new_action.sa_sigaction = handler;
    new_action.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &new_action, nullptr);

    while (true) { sleep(10); }

    return 0;
}

