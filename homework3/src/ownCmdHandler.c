#include <func.h>
#include <myhdr.h>

int ownCmdHandler(char ****cmd, int sockfd) {
    int typeOfCmd = -1;
    char *ListOfCmd[NoOfCmd], *buf = malloc(sizeof(char*));

    ListOfCmd[0] = "quit";
    ListOfCmd[1] = "cd";
    ListOfCmd[2] = "clear";
    ListOfCmd[3] = "setenv";
    ListOfCmd[4] = "printenv";

    for (int i = 0; i < NoOfCmd; i++) {
        if (strcmp((*cmd)[0][0], ListOfCmd[i]) == 0) {
            typeOfCmd = i;
            break;
        }
    }

    switch (typeOfCmd) {
        case 0:
            exit(0);
        case 1:
            chdir((*cmd)[0][1]);
            return 1;
        case 2:
            write(sockfd, "\033[H\033[J", strlen("\033[H\033[J"));
            return 1;
        case 3:
            if ((*cmd)[0][1] == NULL) {
                char msg[] = "setenv needs a name!";
                write(sockfd, msg, strlen(msg));
                return 1;
            }
            buf = getenv((*cmd)[0][1]);
            if (!buf) {
                if (setenv((*cmd)[0][1], (*cmd)[0][2], 1) < 0) {
                    perror("setenv");
                }
            }
            else {
                strcat(buf, ":");
                strcat(buf, (*cmd)[0][2]);
                if (setenv((*cmd)[0][1], buf, 1) < 0) {
                    perror("setenv");
                }
            }
            return 1;
        case 4:
            if ((*cmd)[0][1] == NULL) {
                char msg[] = "getenv needs a name!";
                write(sockfd, msg, strlen(msg));
                return 1;
            }
            buf = getenv((*cmd)[0][1]);
            if (buf) {
                char msg[1024] = {};
                sprintf(msg, "\t%s\n", buf);
                write(sockfd, msg, strlen(msg));
            }
            else write(sockfd, "\n", strlen("\n"));
            return 1;
        default:
            break;
    }

    return 0;
}