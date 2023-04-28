#include <func.h>
#include <myhdr.h>
#include <my_structure.h>

int processingStr(char *str, char ****cmd, struct np **list, int sockfd) {
    char **strPipe;
    strPipe = (char **)malloc(MAX_CMD * sizeof(char *));
    char *temp = malloc(sizeof(char *));
    int typeOfexec = 0;

    typeOfexec = ParsePipe(&str, &strPipe);

    for (int i = 0; i < MAX_CMD; i++) {
        if (strPipe[i] == NULL) break;
        strcpy(temp, strPipe[i]);
        (*cmd)[i] = (char**)malloc(MAX_CMD * sizeof(char *));
        ParseSpace(&temp, &((*cmd)[i]));
    }

    if (typeOfexec == 0) {
        if (checkNumPipe(list)) typeOfexec += 1;
    }
    else if (typeOfexec == 1){
        char *num = strdup((*cmd)[1][0]);
        addNode(&list, (*cmd)[0][0], atoi(num));
        typeOfexec += 1;
    }
    if (ownCmdHandler(cmd, sockfd)) return 0;
    else return 1 + typeOfexec;
}