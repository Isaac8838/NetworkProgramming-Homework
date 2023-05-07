#include <myhdr.h>
#include <func.h>
#include <my_structure.h>

void shell(char *buf, int sockfd, struct np *list) {
    int typeOfExec = 0;
    setenv("PATH", "bin:.:/bin", 1);
    // struct np *list = NULL;
    char ***cmd;
    cmd = (char ***)malloc(MAX_CMD * sizeof(char **));
    for (int i = 0; i < MAX_CMD; i++) {
        cmd[i] = NULL;
    }
    subNum(&list);
    typeOfExec = processingStr(buf, &cmd, &list, sockfd);
    switch(typeOfExec) {
        case 1:
            ExecArgsPipe(cmd, &list, sockfd);
            break;
        case 2:
            struct np **temp = &list;
            while (*temp != NULL) {
                if ((*temp)->num == 0) {
                    ExecArgsPipe(numCmd(&(*temp), cmd), &list, sockfd);
                    if ((*temp)->num == 0) removeNode(&list, &(*temp));
                    temp = &list;
                }
                else temp = &((*temp)->next);
            }
        default:
            break;
    }
}