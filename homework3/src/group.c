#include <myhdr.h>
#include <func.h>
#include <my_structure.h>
#include <my_redis.h>

int isGroup(int sockfd, int s_fifofd, int c_fifofd, char *buf, char *name) {
    char groupCMD[7][12] = {"gyell", "createGroup", "delGroup", "addTo", "leaveGroup", "listGroup", "remove"};
    char cmd[64] = {};
    sscanf(buf, "%s", cmd);
    for (int i = 0; i < 7; i++) {
        if (strcmp(cmd, groupCMD[i]) == 0) {
            if (i == 0) {
                Gyell(sockfd, s_fifofd, c_fifofd, name, buf);
                return 1;
            }
            else if (i == 1) {
                CreateGroup(sockfd, name, buf);
                return 1;
            }
            else if (i == 2) {
                DelGroup(sockfd, name, buf);
                return 1;
            }
            else if (i == 3) {
                AddTo(sockfd, name, buf);
                return 1;
            }
            else if (i == 4) {
                LeaveGroup(sockfd, name, buf);
                return 1;
            }
            else if (i == 5) {
                ListGroup(sockfd, name, buf);
                return 1;
            }
            else if (i == 6) {
                Remove(sockfd, name, buf);
                return 1;
            }
        }
    }
    return 0;
}

void Gyell(int sockfd, int s_fifofd, int c_fifofd, char *name, char *cmd) {
    char groupName[256] = {};
    char message[1024] = {};
    sscanf(cmd, "gyell %s %s", groupName, message);

    reply = redisCommand(rc, "ZRANGE groupList 0 -1");

    int found = 0;
    if (reply && reply->type == REDIS_REPLY_ARRAY) {
        redisReply **tr = reply->element;
        size_t len = reply->elements;
        for (int i = 0; i < len; i++) {
            if (strcmp(tr[i]->str, groupName) == 0) {
                found = 1;
                break;
            }
        }
        freeReplyObject(reply);
        if (found == 0) {
            char msg[] = "Group not found!\n";
            write(sockfd, msg, strlen(msg));
            return;
        }

        char online[MAX_CLIENT][64] = {};
        reply = redisCommand(rc, "ZRANGE %s 0 -1", groupName);
        if (reply && reply->type == REDIS_REPLY_ARRAY) {
            redisReply **r = reply->element;
            len = reply->elements;
            for (int i = 0; i < len; i++) {
                strcpy(online[i], r[i]->str);
            }
        }
        char msg[4096] = {};
        int ack = 0;
        sprintf(msg, "<%s-%s>: %s\n", groupName, name, message);

        // send msg
        write(c_fifofd, "gyell", strlen("gyell"));
        read(s_fifofd, &ack, sizeof(int));
        if (ack)
            write(c_fifofd, msg, strlen(msg));
        read(s_fifofd, &ack, sizeof(int));
        if (ack)
            write(c_fifofd, groupName, strlen(groupName));
    }
    else {
        char msg[] = "Group not found!\n";
        write(sockfd, msg, strlen(msg));
        freeReplyObject(reply);
        return; 
    }
    freeReplyObject(reply);
}

void CreateGroup(int sockfd, char *name, char *cmd) {
    char groupName[256] = {};
    sscanf(cmd, "createGroup %s", groupName);

    reply = redisCommand(rc, "ZCARD groupList");
    int gid = reply->integer;
    freeReplyObject(reply);

    reply = redisCommand(rc, "ZADD groupList %d %s", gid + 1, groupName);

    if (reply->integer) {
        redisReply *tr = redisCommand(rc, "ZADD %s 1 %s", groupName, name);
        freeReplyObject(tr);
        char msg[] = "Create success !\n";
        write(sockfd, msg, strlen(msg));
    }
    else {
        char msg[] = "Group already exist !\n";
        write(sockfd, msg, strlen(msg));
    }
    freeReplyObject(reply);
}

void DelGroup(int sockfd, char *name, char *cmd) {
    char delGroup[64] = {};
    sscanf(cmd, "delGroup %s", delGroup);

    reply = redisCommand(rc, "ZRANGE %s 0 -1", delGroup);

    if (reply && reply->type == REDIS_REPLY_ARRAY) {
        if (reply->elements == 0) {    
            char msg[] = "Group not found !\n";
            write(sockfd, msg, strlen(msg));
            freeReplyObject(reply);
            return;
        }
 
       if (strcmp(reply->element[0]->str, name) == 0) {
            redisReply *tr = redisCommand(rc, "DEL %s", delGroup);
            char msg[] = "Group delete success !\n";
            write(sockfd, msg, strlen(msg));
            freeReplyObject(tr);
            
            tr = redisCommand(rc, "ZREM groupList %s", delGroup);
            freeReplyObject(tr);
        }
        else {
            char msg[] = "You don't have permissions !\n";
            write(sockfd, msg, strlen(msg));   
        }
    }
    else {
           }
    freeReplyObject(reply);
}

void AddTo(int sockfd, char *name, char *cmd) {
    char groupName[256] = {};
    char nameList[100][64] = {};
    char success[100][64] = {};
    char notFound[100][64] = {};
    char inGroup[100][64] = {};
    int pos = 0, count = 0, nextPos = 0;
    int sc = 0, nf = 0, ig = 0;
    redisReply *tr;

    sscanf(cmd, "addTo %s %n", groupName, &pos);

    while (sscanf(cmd + pos, "%s %n", nameList[count], &nextPos) == 1) {
        count++;
        pos += nextPos;
    }

    reply = redisCommand(rc, "ZRANGE %s 0 -1", groupName);
    if (reply->type == REDIS_REPLY_ARRAY) {
        if (reply->elements == 0) {
            char msg[] = "Group Not Found !\n";
            write(sockfd, msg, strlen(msg));
            freeReplyObject(reply);
            return;
        }

        if (strcmp(reply->element[0]->str, name) != 0) {
            char msg[] = "You don't have permissions !\n";
            write(sockfd, msg, strlen(msg));
            freeReplyObject(reply);
            return;
        }

        // user not found            
        for (int i = 0; i < count; i++) {
            tr = redisCommand(rc, "GET %s", nameList[i]);
            if (tr == NULL || tr->type != REDIS_REPLY_STRING) {
                strcpy(notFound[nf++], nameList[i]);
                strcpy(nameList[i], "");
            }
            freeReplyObject(tr);
        }

        // user already in group
        for (int i = 0; i < reply->elements; i++) {
            for (int j = 0; j < count; j++) {
                if (strcmp(reply->element[i]->str, nameList[j]) == 0) {
                    strcpy(inGroup[ig++], nameList[j]);
                    strcpy(nameList[j], "");
                }
            }
        }

        // add user into group
        int gid = reply->elements + 1;
        for (int i = 0; i < count; i++) {
            if (strcmp(nameList[i], "") != 0) {
                tr = redisCommand(rc, "ZADD %s %d %s", groupName, gid++, nameList[i]); 
                strcpy(success[sc++], nameList[i]);
                freeReplyObject(tr);
            }
        }

        char prompt[1024] = {};

        // already in group msg
        for (int i = 0; i < ig; i++) {
            strcat(prompt, inGroup[i]);
            strcat(prompt, " ");
        }
        if (strcmp(prompt, "") != 0) {
            strcat(prompt, "already in group !\n");
            write(sockfd, prompt, strlen(prompt));
        }

        // not found msg
        memset(prompt, 0, sizeof(prompt));
        for (int i = 0; i < nf; i++) {
            strcat(prompt, notFound[i]);
            strcat(prompt, " ");
        }
        if (strcmp(prompt, "") != 0) {
            strcat(prompt, "not found !\n");
            write(sockfd, prompt, strlen(prompt));
        }

        // success msg
        memset(prompt, 0, sizeof(prompt));
        for (int i = 0; i < sc; i++) {
            strcat(prompt, success[i]);
            strcat(prompt, " ");
        }
        if (strcmp(prompt, "") != 0) {
            strcat(prompt, "add success !\n");
            write(sockfd, prompt, strlen(prompt));
        }
    }
    freeReplyObject(reply);
}

void LeaveGroup(int sockfd, char *name, char *cmd) {
    char groupName[256] = {};
    sscanf(cmd, "leaveGroup %s", groupName);

    reply = redisCommand(rc, "ZSCAN groupList 0 match %s", groupName);

    if (!reply->element[1]->element) {
        char msg[] = "Group not found !\n";
        write(sockfd, msg, strlen(msg));
        freeReplyObject(reply);
        return;
    }

    freeReplyObject(reply);
    redisReply *tr = redisCommand(rc, "ZRANGE %s 0 -1", groupName);
    size_t numberOfpeople = tr->elements;
    freeReplyObject(tr);
    reply = redisCommand(rc, "ZREM %s %s", groupName, name);

    if (reply && reply->integer == 0) {
        char msg[] = "Leave fault !\n";
        write(sockfd, msg, strlen(msg));
        freeReplyObject(reply);
        return;
    }

    
    if (numberOfpeople == 1) {
        tr = redisCommand(rc, "ZREM groupList %s", groupName);
        freeReplyObject(tr);
    }

    char msg[] = "Leave accept !\n";
    write(sockfd, msg, strlen(msg));
    freeReplyObject(reply);

}

void ListGroup(int sockfd, char *name, char *cmd) {
    reply = redisCommand(rc, "ZRANGE groupList 0 -1");

    char prompt[] = " <owner> \t <group>  \n";
    int flag = 0;

    if (reply && reply->type == REDIS_REPLY_ARRAY) {
        size_t len = reply->elements;
        redisReply **r = reply->element;
        redisReply *reply_nest;
        char owner[64] = {};
        char group[64] = {};
        int first = 1;
        for (int i = 0; i < len; i++) {
            reply_nest = redisCommand(rc, "ZRANGE %s 0 -1", r[i]->str);
            if (reply_nest && reply_nest->type == REDIS_REPLY_ARRAY) {
                size_t lrn = reply_nest->elements;
                if (lrn > 0) {
                    redisReply **rn = reply_nest->element;
                    memset(group, 0, sizeof(group));
                    memset(owner, 0, sizeof(owner));
                    strcpy(group, r[i]->str);
                    strcpy(owner, rn[0]->str);
                    char msg[256] = {};
                    sprintf(msg, " %s \t %s  \n", owner, group);
                    for (int j = 0; j < lrn; j++) {
                        if (strcmp(rn[j]->str, name) == 0) {
                            if (first) {
                                write(sockfd, prompt, strlen(prompt));
                                first = 0;
                            }
                            flag = 1;
                            write(sockfd, msg, strlen(msg));
                            break;
                        }
                    }
                }
            }
            freeReplyObject(reply_nest);
        }
    }

    if (flag == 0) {
        write(sockfd, "Empty !\n", strlen("Empty !\n"));
    }

    freeReplyObject(reply);
}

void Remove(int sockfd, char *name, char *cmd) {
    char groupName[256] = {};
    char nameList[100][64] = {};
    char success[100][64] = {};
    char notFound[100][64] = {};
    char notInGroup[100][64] = {};
    int pos = 0, count = 0, nextPos = 0;
    int sc = 0, nf = 0, nig = 0;

    sscanf(cmd, "remove %s %n", groupName, &pos);

    while (sscanf(cmd + pos, "%s %n", nameList[count], &nextPos) == 1) {
        count++;
        pos += nextPos;
    }

    reply = redisCommand(rc, "ZRANGE %s 0 -1", groupName);

    if (reply && reply->type == REDIS_REPLY_ARRAY) {
        redisReply *tr;
        if (reply->elements == 0) {
            char msg[] = "Group not found!\n";
            write(sockfd, msg, strlen(msg));
            freeReplyObject(reply);
            return;
        }

        if (strcmp(reply->element[0]->str, name) != 0) {
            char msg[] = "You don't have permissions !\n";
            write(sockfd, msg, strlen(msg));
            freeReplyObject(reply);
            return;
        }
        
        // user not exist
        for (int i = 0; i < count; i++) {
            tr = redisCommand(rc, "GET %s", nameList[i]);
            if (tr == NULL || tr->type != REDIS_REPLY_STRING) {
                strcpy(notFound[nf++], nameList[i]);
                strcpy(nameList[i], "");
            }
            freeReplyObject(tr);
        }

        // user not in group
        for (int i = 0; i < count; i++) {
            int flag = 0;
            for (int j = 0; j < reply->elements; j++) {
                if (strcmp(reply->element[j]->str, nameList[i]) == 0) {
                    flag = 1;
                    break;
                }
            }
            if (!flag) {
                strcpy(notInGroup[nig++], nameList[i]);
                strcpy(nameList[i], "");
            }
        }

        // remove user
        int gid = reply->elements + 1;
        for (int i = 0; i < count; i++) {
            if (strcmp(nameList[i], "") != 0) {
                tr = redisCommand(rc, "ZREM %s %s", groupName, nameList[i]);
                strcpy(success[sc++], nameList[i]);
                freeReplyObject(tr);
            }
        }

        // user not found msg
        char prompt[1024] = {};
        write(sockfd, &nf, sizeof(int));
        for (int i = 0; i < nf; i++) {
            strcat(prompt, notFound[i]);
            strcat(prompt, " ");
        }
        if (strcmp(prompt, "") != 0) {
            strcat(prompt, "not found !\n");
            write(sockfd, prompt, strlen(prompt));
        }

        // user not in group
        memset(prompt, 0, sizeof(prompt));
        for (int i = 0; i < nig; i++) {
            strcat(prompt, notInGroup[i]);
            strcat(prompt, " ");
        }
        if (strcmp(prompt, "") != 0) {
            strcat(prompt, "not in group !\n");
            write(sockfd, prompt, strlen(prompt));
        }

        // user remove success
        memset(prompt, 0, sizeof(prompt));
        for (int i = 0; i < sc; i++) {
            strcat(prompt, success[i]);
            strcat(prompt, " ");
        }
        if (strcmp(prompt, "") != 0) {
            strcat(prompt, "remove success !\n");
            write(sockfd, prompt, strlen(prompt));
        }
    }
   
    freeReplyObject(reply);
}