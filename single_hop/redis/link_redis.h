#ifndef __LINK_FS_H__
#define __LINK_FS_H__

#include "hircluster.h"

void genNodeList(char *nodelist);
redisClusterContext *connectToRedisServer();
void disconnectFromRedisServer(redisClusterContext *cc);
void create_return_link_redis(redisClusterContext *cc, char *key_str, char *target);
void create_link_redis(redisClusterContext *cc, char *key_str);

#endif
