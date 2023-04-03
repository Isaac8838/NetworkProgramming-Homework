#include <myhdr.h>
#include <my_structure.h>
#include <func.h>

void init_list(struct c_info (*list)[MAX_CLIENT]) {
    for (int i = 0; i < MAX_CLIENT; i++) {
        (*list)[i].user_id = -1;
    }
}

void addClient(struct c_info (*list)[MAX_CLIENT], struct c_info *newClient) {
    int index = newClient->user_id;
    (*list)[index].user_id = newClient->user_id;
    // (*list)[index].user_name = newClient->user_name;
    strcpy((*list)[index].user_name , newClient->user_name);
    (*list)[index].user_ip = newClient->user_ip;
    (*list)[index].user_port = newClient->user_port;
}

int changeC_Name(struct c_info (*list)[MAX_CLIENT], int id, char *newName) {
    for (int i = 0; i < MAX_CLIENT; i++) {
        if ((*list)[i].user_id == -1) continue;
        if (strcmp((*list)[i].user_name, newName) == 0) return 0;
    }
    // (*list)[id].user_name = newName;
    // (*list)[id].user_name = strdup(newName);
    strcpy((*list)[id].user_name , newName);
    return 1;
}

int getMinC_ID(struct c_info list[MAX_CLIENT]) {
    for (int i = 0; i < MAX_CLIENT; i++) {
        if (list[i].user_id == -1) return i;
    }
    return -1;
}