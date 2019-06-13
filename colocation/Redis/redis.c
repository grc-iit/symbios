//
// Created by kfeng on 4/18/19.
//

#include <stdlib.h>
#include <string.h>
#include "redis.h"

char *redis_node_base = "ares-stor";
int redis_port_base = 7000;
int redis_node_id_base = 16;
int redis_node_count = 16;
char *redis_node_postfix = "-40g";

redisClusterContext *cc;

char *randstring(long length) {
  long n;
  static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  char *randomString = NULL;
  if (length) {
    randomString = (char *) malloc(sizeof(char) * (length + 1));
    if (randomString) {
      for (n = 0; n < length; n++) {
        int key = rand() % (int) (sizeof(charset) - 1);
        randomString[n] = charset[key];
      }
      randomString[length] = '\0';
    }
  }
  return randomString;
}

int redis_init(int rank)
{
  char nodelist[MAX_NODELIST_LEN];
  char tmp[32];
  int i;

  memset(nodelist, 0, MAX_NODELIST_LEN);
  for (i = 0; i < redis_node_count; i++) {
    if (i > 1)
      sprintf(tmp, "%s-%02d%s:%d", redis_node_base, redis_node_id_base + i + 1, redis_node_postfix, redis_port_base + i);
    else
      sprintf(tmp, "%s-%02d%s:%d", redis_node_base, redis_node_id_base + i, redis_node_postfix, redis_port_base + i);
    strcat(nodelist, tmp);
    if (i != redis_node_count - 1)
      strcat(nodelist, ",");
  }
  if (rank == 0) printf("Redis node list: %s\n", nodelist);
  cc = redisClusterContextInit();
  redisClusterSetOptionAddNodes(cc, nodelist);
  redisClusterConnect2(cc);
  if (cc != NULL && cc->err) {
    printf("Fail to connect to Redis server: %s\n", cc->errstr);
    exit(-1);
  } else
    return 0;
}

int prepare_data_redis(char *trace_file, int rank)
{
  /*Initialization of some stuff*/
  FILE *trace;
  char *line = NULL;
  size_t len = 0;
  ssize_t readsize;
  char *operation;
  long offset = 0;
  size_t request_size = 0;
  char *word;
  char *writebuf, *key;
  redisReply *reply;

  /*Opening the trace file*/
  trace = fopen(trace_file, "r");
  if (trace == NULL) {
    return -1;
  }

  /*While loop to read each line from the trace and create I/O*/
  int lineNumber = 0;

  while ((readsize = getline(&line, &len, trace)) != -1) {
    if (readsize < 4) {
      break;
    }
    word = strtok(line, ",");
    operation = word;
    word = strtok(NULL, ",");
    offset = atol(word);
    word = strtok(NULL, ",");
    request_size = atol(word);
    if (!strcmp(operation, "FOPEN")) {
    } else if (!strcmp(operation, "FCLOSE")) {
    } else if (!strcmp(operation, "WRITE")) {
    } else if (!strcmp(operation, "READ")) {
      if (request_size == 0) continue;
      writebuf = randstring(request_size);
      key = (char *)malloc(MAX_KEY_LEN);
      memset(key, 0, MAX_KEY_LEN);
      sprintf(key, "%d_%d_%d", offset, request_size, rank);
//      printf("Putting key: %s, value: %s\n", key, writebuf);
      reply = (redisReply *)redisClusterCommand(cc, "SET %s %b", key, writebuf, (size_t)request_size);
      if (cc != NULL && cc->err) {
        printf("Fail to put data: %s\n", cc->errstr);
        exit(-1);
      }
      if (strcmp(reply->str, "OK")) {
        printf("Fail to put key: %s\n", key);
        exit(-1);
      }
      if (writebuf) free(writebuf);
      if (key) free(key);
    } else if (!strcmp(operation, "LSEEK")) {
    }
    lineNumber++;
  }

  if (line) free(line);

  fclose(trace);

  return 0;
}

size_t replay_trace_redis(char *trace_file, int rank)
{
  /*Initialization of some stuff*/
  FILE *trace;
  char *line = NULL;
  size_t len = 0;
  ssize_t readsize;
  char *operation;
  long offset = 0;
  size_t request_size = 0, total_size = 0;
  char *word;
  char *writebuf, *key;
  redisReply *reply;

  /*Opening the trace file*/
  trace = fopen(trace_file, "r");
  if (trace == NULL) {
    return -1;
  }

  /*While loop to read each line from the trace and create I/O*/
  int lineNumber = 0;

  while ((readsize = getline(&line, &len, trace)) != -1) {
    if (readsize < 4) {
      break;
    }
    word = strtok(line, ",");
    operation = word;
    word = strtok(NULL, ",");
    offset = atol(word);
    word = strtok(NULL, ",");
    request_size = atol(word);
    total_size += request_size;
    if (!strcmp(operation, "FOPEN")) {
    } else if (!strcmp(operation, "FCLOSE")) {
    } else if (!strcmp(operation, "WRITE")) {
      if (request_size == 0) continue;
      writebuf = randstring(request_size);
      key = (char *)malloc(MAX_KEY_LEN);
      memset(key, 0, MAX_KEY_LEN);
      sprintf(key, "%d_%d_%d", offset, request_size, rank);
//      printf("Putting key: %s\n", key);
      reply = (redisReply *)redisClusterCommand(cc, "SET %s %b", key, writebuf, (size_t)request_size);
      if (cc != NULL && cc->err) {
        printf("Fail to put data: %s\n", cc->errstr);
        exit(-1);
      }
      if (strcmp(reply->str, "OK")) {
        printf("Fail to put key: %s\n", key);
        exit(-1);
      }
      freeReplyObject(reply);
      if (writebuf) free(writebuf);
      if (key) free(key);
    } else if (!strcmp(operation, "READ")) {
      key = (char *)malloc(MAX_KEY_LEN);
      memset(key, 0, MAX_KEY_LEN);
      sprintf(key, "%d_%d_%d", offset, request_size, rank);
      /*printf("Getting key: %s\n", key);*/
      reply = (redisReply *)redisClusterCommand(cc, "GET %s", key);
      if (reply->len != request_size) {
        printf("Something is wrong during replaying read\n");
        exit(-1);
      }
      if (key) free(key);
      freeReplyObject(reply);
    } else if (!strcmp(operation, "LSEEK")) {
    }
    lineNumber++;
  }

  if (line) free(line);

  fclose(trace);

  return total_size;
}

void redis_finalize(int rank)
{
  return redisClusterFree(cc);
}
