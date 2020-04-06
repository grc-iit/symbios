#ifndef __IO_REDIS_H__
#define __IO_REDIS_H__

#include "hircluster.h"

void genNodeList(char *nodelist);
void genNodeListFromHostfile(char *hostfile, char *nodelist);
redisClusterContext *connectToRedisServer();
redisClusterContext *connectToRedisServerWithHostfile(char *hostfile);
void disconnectFromRedisServer(redisClusterContext *cc);
void write_redis(redisClusterContext *cc, char *key_str, char *value);
void read_redis(redisClusterContext *cc, char *key_str, char *value);
void write_redis_sized(redisClusterContext *cc,
                       char *key_str,
                       char *value,
                       int length);
void read_redis_sized(redisClusterContext *cc,
                      char *key_str,
                      char *value,
                      int length);

#endif
