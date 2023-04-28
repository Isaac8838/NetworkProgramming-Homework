#include <myhdr.h>
#include <func.h>
#include <my_structure.h>
#include <my_redis.h>

char *login_logic(int sockfd) {
    char choose[] = "Create account or login again ? <1/2> : ";
    char unExpected[] = "unExpected input number !\n";
    char *num = malloc(sizeof(char) * 1024);
    write(sockfd, choose, strlen(choose));
    read(sockfd, num, 1024);
    for (int i = 0; i < strlen(num); i++) {
        if (num[i] == '\r' || num[i] == '\n') 
            num[i] = '\0';
    }
    int register_or = atoi(num);
    switch(register_or) {
        case 1:
            return register_user(sockfd);
        case 2:
            return login(sockfd);
        default:
            write(sockfd, unExpected, strlen(unExpected));
            return login_logic(sockfd);
    }
}

char *register_user(int sockfd) {
    char userName[] = "your username: ";
    char password[] = "your password: ";
    char *name = malloc(sizeof(char) * 1024);
    char *pwd = malloc(sizeof(char) * 1024);

    write(sockfd, userName, strlen(userName));
    read(sockfd, name, 1024);
    write(sockfd, password, strlen(password));
    read(sockfd, pwd, 1024);

    for (int i = 0; i < strlen(name); i++) {
        if (name[i] == '\r' || name[i] == '\n') 
            name[i] = '\0';
    }
    for (int i = 0; i < strlen(pwd); i++) {
        if (pwd[i] == '\r' || pwd[i] == '\n') 
            pwd[i] = '\0';
    }

    char exists[] = "Error: Username already exists\n";
    char FailinRedis[] = "Error: Failed to set username and password in Redis\n";
    char Succece[] = "Create Succece!\n";

    reply = (redisReply*)redisCommand(rc, "GET %s", name);
    while (reply != NULL && reply->type == REDIS_REPLY_STRING) {
        write(sockfd , exists , strlen(exists));
        freeReplyObject(reply);
        return register_user(sockfd);
    }

    freeReplyObject(reply);    

    reply = (redisReply*)redisCommand(rc, "SET %s %s", name, pwd);
    if (reply == NULL) {
        write(sockfd , FailinRedis , strlen(FailinRedis));
        return NULL;
    }
    freeReplyObject(reply);

    write(sockfd , Succece , strlen(Succece));

    reply = (redisReply*)redisCommand(rc, "LPUSH user_list %s", name);

    return login(sockfd);
}

char *login(int sockfd) {
    char userName[] = "user_name: ";
    char password[] = "password: ";
    char *name = malloc(sizeof(char) * 1024);
    char *pwd = malloc(sizeof(char) * 1024);

    char notFound[] = "User not found !\n";
    char passError[] = "Password error !\n";

    write(sockfd, userName, strlen(userName));
    read(sockfd, name, 1024);
    write(sockfd, password, strlen(password));
    read(sockfd, pwd, 1024);
    
    for (int i = 0; i < strlen(name); i++) {
        if (name[i] == '\r' || name[i] == '\n') 
            name[i] = '\0';
    }
    for (int i = 0; i < strlen(pwd); i++) {
        if (pwd[i] == '\r' || pwd[i] == '\n') 
            pwd[i] = '\0';
    }

    reply = (redisReply*)redisCommand(rc, "GET %s", name);
    if (reply == NULL || reply->type != REDIS_REPLY_STRING) {
        write(sockfd, notFound, strlen(notFound));
        freeReplyObject(reply);
        return login_logic(sockfd);
    }

    int result = strcmp(reply->str, pwd);

    freeReplyObject(reply);

    if (result == 0) {
        return name;
    } else {
        write(sockfd , passError , strlen(passError));
        return login(sockfd);
    }
}