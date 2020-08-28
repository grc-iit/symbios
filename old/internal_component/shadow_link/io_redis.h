#ifndef __IO_REDIS_H__
#define __IO_REDIS_H__

#include "hircluster.h"
#include "common.h"

void genNodeList(char *nodelist);
void genNodeListFromHostfile(char *hostfile, char *nodelist);
int connectToRedisServer();
int connectToRedisServerWithHostfile(char *hostfile);
void disconnectFromRedisServer();
void write_redis(char *key_str, char *value);
void read_redis(char *key_str, char *value);
void write_redis_sized(char *key_str,
                       char *value,
                       int length);
void read_redis_sized(char *key_str,
                      char *value,
                      int length);
void create_symbios_metadata_redis(symbios_metadata *metadata);
symbios_metadata *query_symbios_metadata_by_id_redis(char *data_id);

#endif
