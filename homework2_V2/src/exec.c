#include <myhdr.h>
#include <func.h>
#include <my_structure.h>

void ExecArgsPipe(char ***cmd, struct np **list, int sockfd) {
    int fd[2];
    pid_t pid;
    int fdd = 0;
    int status;

    while (*cmd != NULL) {
        pipe(fd);
        if ((pid = fork()) < 0) {
            perror("fork error");
            return;
        }
        else if (pid == 0) {
            dup2(fdd, 0);
            if (*(cmd + 1) != NULL) {
                dup2(fd[1], STDOUT_FILENO);
            }
            if (*(cmd + 1) == NULL) {
                dup2(sockfd, STDOUT_FILENO);
            }
            close(fd[0]);
            if ((execvp((*cmd)[0], *cmd)) < 0) {
                char buf[64];
                // fprintf(stderr, "UnKnown command: [%s].\n", *(cmd)[0]);
                sprintf(buf, "UnKnown command: [%s].\n", *(cmd)[0]);
                write(sockfd, buf, strlen(buf));
                exit(EXIT_FAILURE);
            }
        }
        else {
            waitpid(pid, &status, 0);
            if ((WIFEXITED(status) && WEXITSTATUS(status) == EXIT_FAILURE)) {
                addNum(&(*list));
            }
            close(fd[1]);
            fdd = fd[0];
            cmd++;
        }
    }
}