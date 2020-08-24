#include <string.h>
#include <stdlib.h>
#include "../xattr/common.h"
#include "link_redis.h"

void genNodeList(char *nodelist)
{
  int i;
  char tmp[32];

  memset(nodelist, 0, 1024);
  for (i = 0; i < REDIS_HOST_COUNT; i++) {
    sprintf(tmp, "%s%02d:%d", REDIS_HOST_BASE, REDIS_HOST_START + i, REDIS_PORT_BASE + i);
    strcat(nodelist, tmp);
    if (i != REDIS_HOST_COUNT - 1)
      strcat(nodelist, ",");
  }
}

redisClusterContext *connectToRedisServer()
{
  char nodelist[1024];
  redisClusterContext *cc = (redisClusterContext *)malloc(sizeof(redisClusterContext));

  genNodeList(nodelist);
  cc = redisClusterContextInit();
  /*printf("Node list: %s\n", nodelist);*/
  redisClusterSetOptionAddNodes(cc, nodelist);
  redisClusterConnect2(cc);
  if (cc != NULL && cc->err) {
    printf("Error: %s\n", cc->errstr);
    // handle error
  }
  return cc;
}

void disconnectFromRedisServer(redisClusterContext *cc)
{
  redisClusterReset(cc);
  redisClusterFree(cc);
}

void create_return_link_redis(redisClusterContext *cc, char *key_str, char *target)
{
  redisReply *reply;

  /*printf("%s\n", key_str);*/
  reply = redisClusterCommand(cc, "SET %s %b", key_str, target, strlen(target));
  if (strcmp(reply->str, "OK"))
    printf("Something is wrong\n");
  freeReplyObject(reply);
}

void create_link_redis(redisClusterContext *cc, char *key_str)
{
  redisReply *reply;
  char *value = "redis";

  /*printf("%s\n", key_str);*/
  reply = redisClusterCommand(cc, "SET %s %b", key_str, value, strlen(value));
  if (strcmp(reply->str, "OK"))
    printf("Something is wrong\n");
  freeReplyObject(reply);
}
