#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <pwd.h>
#include <sys/ucontext.h>

void handler(int, siginfo_t* info, void* context) {
    char buf[256];

    int len = snprintf(buf, sizeof(buf), 
            "Received SIGUSR1 from process %d executed by %d (%s).\n", 
            info->si_pid, info->si_uid, getpwuid(info->si_uid)->pw_name );
    write(STDOUT_FILENO, buf, len);

    ucontext_t* uctx = (ucontext_t*)context;
    len = snprintf(buf, sizeof(buf),
            "State of the context: X0 = %llx, X1 = %llx, PC = %llx\n",
            uctx->uc_mcontext.regs[0], uctx->uc_mcontext.regs[1], uctx->uc_mcontext.pc);
    write(STDOUT_FILENO, buf, len);
}

int main() {
    char buf[64];
    int len = snprintf(buf, sizeof(buf), "PID: %d\n", getpid());
    write(STDOUT_FILENO, buf, len);

    struct sigaction sa = {};
    sa.sa_sigaction = handler;
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &sa, nullptr);

    while (true) { sleep(10); }

    return 0;
}
