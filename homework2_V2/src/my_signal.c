#include <myhdr.h>
#include <func.h>

void sigchld_handler(int sig) {
    int save = errno;
    while (waitpid((pid_t)(-1), 0, WNOHANG) > 0) {}
    errno = save;
}

void set_signal_action() {
    struct sigaction act;
    bzero(&act, sizeof(act));
    act.sa_handler = &sigchld_handler;
    sigaction(SIGCHLD, &act, NULL);
}