#include <myhdr.h>
#include <func.h>

int ParsePipe(char **str, char ***pipeStr) {
    int i;
    char *temp = strdup(*str);
    for (i = 0; i < BUF_SIZE; i++) {
        (*pipeStr)[i] = strsep(&temp, "|");
        if ((*pipeStr)[i] == NULL) break;
    }
    
    if (isNum((*pipeStr)[i - 1])) {
        return 1;
    }
    return 0;
}

void ParseSpace(char **str, char ***cmd) {
    int i = 0;
    char *temp = strdup(*str);
    for (int i = 0; i < MAX_CMD; i++) {
        if (((*cmd)[i] = strsep(&temp, " ")) == NULL) break;
        if (strlen((*cmd)[i]) == 0) i--;
    }
}

bool isNum(char *str) {
    int len = strlen(str);
    for (int i = 0; i < len; i++) {
        if (!isdigit(str[i])) return false;
    }
    return true;
}