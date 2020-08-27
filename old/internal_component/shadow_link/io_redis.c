#include <string.h>
#include <stdlib.h>
#include "common.h"
#include "io_redis.h"

redisClusterContext *cc = NULL;

void genNodeList(char *nodelist)
{
  int i;
  char tmp[32];

  for (i = 0; i < REDIS_HOST_COUNT; i++) {
    sprintf(tmp, "%s%02d:%d", REDIS_HOST_BASE,
                              REDIS_HOST_START + i,
                              REDIS_PORT_BASE + i);
    strcat(nodelist, tmp);
    if (i != REDIS_HOST_COUNT - 1)
      strcat(nodelist, ",");
  }
}

void genNodeListFromHostfile(char *hostfile, char *nodelist)
{
  int i;
  char tmp[128], node[128];
  FILE *hostfile_fp;

  if (hostfile == NULL) {
    fprintf(stderr, "Invalid Redis hostfile\n");
    exit(-1);
  }

  if ((hostfile_fp = fopen(hostfile, "r")) == NULL) {
    fprintf(stderr, "Fail to open hostfile %s\n", hostfile);
    exit(-1);
  }
  rewind(hostfile_fp);
  for (i = 0; fgets(node, 128, hostfile_fp) != NULL; i++) {
    debug_print(2, "%d-th node: %s", i, node);
    node[strlen(node) - 1] = 0;
    sprintf(tmp, "%s:%d,", node, REDIS_PORT_BASE + i);
    strcat(nodelist, tmp);
  }
  nodelist[strlen(nodelist) - 1] = 0;
  fclose(hostfile_fp);
}

int connectToRedisServer()
{
  char nodelist[2048];
  cc = (redisClusterContext *) malloc(sizeof(redisClusterContext));

  memset(nodelist, 0, 2048);
  genNodeList(nodelist);
  debug_print(1, "Node list: %s\n", nodelist);
  cc = redisClusterContextInit();
  redisClusterSetOptionAddNodes(cc, nodelist);
  redisClusterConnect2(cc);
  if (cc != NULL && cc->err) {
    fprintf(stderr, "Error: %s\n", cc->errstr);
    // handle error
    return -1;
  }
  return 0;
}

int connectToRedisServerWithHostfile(char *hostfile)
{
  char nodelist[2048];
  cc = (redisClusterContext *) malloc(sizeof(redisClusterContext));

  memset(nodelist, 0, 2048);
  genNodeListFromHostfile(hostfile, nodelist);
  debug_print(1, "Node list: %s\n", nodelist);
  cc = redisClusterContextInit();
  redisClusterSetOptionAddNodes(cc, nodelist);
  redisClusterConnect2(cc);
  if (cc != NULL && cc->err) {
    fprintf(stderr, "Error: %s\n", cc->errstr);
    // handle error
    return -1;
  }
  return 0;
}

void disconnectFromRedisServer()
{
  redisClusterReset(cc);
  redisClusterFree(cc);
}

void write_redis(char *key_str, char *value)
{
  redisReply *reply;

  reply = redisClusterCommand(cc, "SET %s %b", key_str, value, strlen(value));
  if (strcmp(reply->str, "OK"))
    fprintf(stderr, "Something is wrong in write\n");
  freeReplyObject(reply);
}

void read_redis(char *key_str, char *value)
{
  redisReply *reply;

  reply = redisClusterCommand(cc, "GET %s", key_str);
  if (reply == NULL)
    fprintf(stderr, "Something is wrong in read\n");
  value = reply->str;
  printf("res: %s\n", reply->str);
  freeReplyObject(reply);
}

void write_redis_sized(char *key_str,
                       char *value,
                       int length)
{
  redisReply *reply;

  reply = redisClusterCommand(cc, "SET %s %b", key_str, value, length);
  if (strcmp(reply->str, "OK"))
    printf("Something is wrong in write\n");
  freeReplyObject(reply);
}

void read_redis_sized(char *key_str,
                      char *value,
                      int length)
{
  redisReply *reply;

  reply = redisClusterCommand(cc, "GET %s", key_str);
  if (reply == NULL)
    printf("Something is wrong in read\n");
  strcpy(value, reply->str);
  debug_print(1, "res: %s\n", reply->str);
  freeReplyObject(reply);
}

void create_symbios_metadata_redis(symbios_metadata *metadata)
{
  char *metadata_str;

  metadata_str = serialize_metadata(metadata);
  write_redis_sized(metadata->data_id, metadata_str, strlen(metadata_str));
}

symbios_metadata *query_symbios_metadata_by_id_redis(char *data_id)
{
  symbios_metadata *metadata;
  char metadata_str[1024];

  read_redis_sized(data_id, metadata_str, 1024);
  metadata = deserialize_metadata(metadata_str);

  return metadata;
}
