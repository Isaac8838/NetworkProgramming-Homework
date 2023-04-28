#include <func.h>
#include <my_redis.h>
#include <myhdr.h>

redisReply *reply = NULL;
redisContext *rc = NULL;

void initRedis() {
    
    rc = redisConnect("127.0.0.1", 6379);
    reply = redisCommand(rc,"PING %s", "Connect Redis Succeced!");
    
    if (rc->err) {
        printf("error: %s\n", rc->errstr);
        return;
    }

    printf("RESPONSE: %s\n", reply->str);

    freeReplyObject(reply);  
}
