#ifndef REDIS_PU_H_INCLUDED
#define REDIS_PU_H_INCLUDED

#include <hiredis/hiredis.h>

redisContext* redis_connect();
void change_server_connect(int serverid, int connect_count);

#endif // REDIS_PU_H_INCLUDED
