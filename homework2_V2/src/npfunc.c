#include <func.h>
#include <myhdr.h>
#include <my_structure.h>

void addNode(struct np ***list, char *str, int n) {
    struct np *newNode = malloc(sizeof(struct np));
    newNode->cmd = strdup(str);
    newNode->num = n;
    newNode->next = NULL;
    struct np **indirect = *list;
    if (*indirect == NULL) {
        *indirect = newNode;
    }
    else {
        while ((*indirect)->next != NULL) {
            indirect = &((*indirect)->next);
        }
        (*indirect)->next = newNode;
    }
}

void removeNode(struct np **list, struct np **target) {
    struct np **indirect = &(*list);
    while ((*indirect) != *target) {
        indirect = &((*indirect)->next);
    }
    *indirect = (*target)->next;
}

char ***numCmd(struct np **target, char ***cmd) {
    char ***newCmd;
    newCmd = (char***)malloc(MAX_CMD * sizeof(char**));
    for (int i = 0; i < MAX_CMD; i++) {
        newCmd[i] = NULL;
    }
    char *temp[2] = {(*target)->cmd, NULL};
    newCmd[0] = temp;
    for (int i = 0; i < MAX_CMD; i++) {
        if (cmd[i] == NULL) break;
        newCmd[i + 1] = cmd[i];
    }
    return newCmd;
}

bool checkNumPipe(struct np **list) {
    struct np **temp = &(*list);
    while (*temp != NULL) {
        if ((*temp)->num == 0) return true;
        temp = &((*temp)->next);
    }
    return false;
}

void subNum(struct np **list) {
    struct np **indirect = &(*list);
    while (*indirect != NULL) {
        if ((*indirect)->num != 0) {
            (*indirect)->num -= 1;
        }
        indirect = &((*indirect)->next);
    }
}

void addNum(struct np **list) {
    struct np **indirect = &(*list);
    while (*indirect != NULL) {
        (*indirect)->num += 1;
        indirect = &((*indirect)->next);
    }
}