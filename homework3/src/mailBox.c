#include <func.h>
#include <myhdr.h>
#include <my_structure.h>
#include <my_redis.h>

int isMailBox(int sockfd, char *buf, char *name) {
    char mailCMD[3][11] = {"listMail", "mailto", "delMail"};
    char cmd[64] = {};
    sscanf(buf, "%s", cmd);

    for (int i = 0; i < 3; i++) {
        if (strcmp(mailCMD[i], cmd) == 0) {
            if (i == 0) {
                ListMail(sockfd, name);
                return 1;
            }
            else if (i == 1) {
                Mailto(sockfd, name, buf);
                return 1;
            }
            else if (i == 2) {
                DelMail(sockfd, name, buf);
                return 1;
            }
        }
    }
    return 0;
}

void ListMail(int sockfd, char *name) {
    reply = redisCommand(rc , "LRANGE %snoMailBox 0 -1", name);
    if (reply) {
        if (reply->type == REDIS_REPLY_ARRAY && reply->elements != 0) {
            char prompt[] = "<id>\t<date>\t\t\t<sender>\t<message>\n";
            write(sockfd, prompt , strlen(prompt));
            redisReply **r = reply->element;
            size_t len = reply->elements;
            for(int i = 0 ; i < len ; i++) {
                write(sockfd, r[i]->str , strlen(r[i]->str));
            }
        } 
        else if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 0) {
            write(sockfd, "empty !\n", strlen("empty !\n"));
        }
    }
    
    freeReplyObject(reply);
}

void Mailto(int sockfd, char *name, char *cmd) {
    char buf[4096] = {};
    struct tm *tm = localtime(&t);
    char current_time[256] = {};
    char message[3072] = {};
    char reciver[64] = {};
    
    char identifier[64+10] = {};

    char notFound[] = "User not found !\n";

    strftime(current_time , sizeof(current_time) , "%Y-%m-%d %H:%M:%S" , tm);
    sscanf(cmd, "mailto %s %[^\n]s", reciver , message);

    reply = (redisReply*)redisCommand(rc, "GET %s", reciver);
    if (reply == NULL || reply->type != REDIS_REPLY_STRING) {
        write(sockfd, notFound, strlen(notFound));
        freeReplyObject(reply);
        return;
    }
    freeReplyObject(reply);

    if (message[0] == '<') {
        sscanf(message , "< %[^\n^\r]s" , message);
        int fd[2];

        if (pipe(fd) == -1)
            perror("pipe");

        shell(message, fd[1]);
        close(fd[1]);
        read(fd[0], message, sizeof(message));
        close(fd[0]);
    }

    sprintf(identifier , "%snoMailBox" , reciver);
    
    reply = redisCommand(rc, "LLEN %s", identifier);
    long long id = 1;
    if(reply && reply->type == REDIS_REPLY_INTEGER)
        id = reply->integer;
    freeReplyObject(reply);

    sprintf(buf , " %lld\t%s   %s \t\t%s\n" , id, current_time, name, message);

    reply = redisCommand(rc, "LPUSH %s %s", identifier, buf);
    if(reply && reply->type == REDIS_REPLY_INTEGER && reply->integer != 0) {
        write(sockfd , "Send accept!\n" , strlen("Send accept!\n"));
    }

    freeReplyObject(reply);

}

void DelMail(int sockfd, char *name, char *cmd) {
    char id[4];
    sscanf(cmd, "%s %s", id, id);

    reply = redisCommand(rc , "LRANGE %snoMailBox 0 -1", name);
    if(reply && reply->type == REDIS_REPLY_ARRAY) {
            
        redisReply **r = reply->element;
        size_t len = reply->elements;
        int i = 0 ;
        char delMsg[4096];
        char check[4];
        for(int i = 0 ; i < len ; i++) {
            sscanf(r[i]->str , " %s", check);
            if(strcmp(check , id) == 0) {
                strcpy(delMsg , r[i]->str);
                break;
            }   
        }
        freeReplyObject(reply);
        reply = redisCommand(rc, "LREM %snoMailBox 0 %s", name, delMsg);
        if(reply->integer <= 0) {
            char msg[] = "Mail id unexist!\n";
            write(sockfd, msg, strlen(msg));
        } else {
            char msg[] = "Delete accept!\n";
            write(sockfd, msg, strlen(msg)); 
        }
        freeReplyObject(reply);

    }
}