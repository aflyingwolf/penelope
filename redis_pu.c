#include <stdio.h>
#include <stdlib.h>
#include <hiredis/hiredis.h>
#include "configs.h"
#include "redis_pu.h"

redisContext* redis_connect()
{
    redisContext* c = redisConnect((char*)REDIS_IP, REDIS_PORT);
    if(c->err)
    {
        redisFree(c);
        return NULL;
    }
    return c;
}

void change_server_connect(int serverid, int connect_count)
{
    redisContext* c = redis_connect();
    char command[256];
    sprintf(command, "INCRBY server_connects_%d %d", serverid, connect_count);
    redisReply* r = (redisReply*)redisCommand(c, command);
    int f = 0;
    if(NULL == r)
    {
        f = -1;
    }
    else if(r->type != REDIS_REPLY_INTEGER)
    {
        f = -2;
    }

    if(f < 0)
    {
        printf("Execute command failure: %s\n", command);
        if(f == -2)
        {
            freeReplyObject(r);
        }
    }
    redisFree(c);
    return;
}
