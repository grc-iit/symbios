#ifndef __COMMON_H__
#define __COMMON_H__

#include <sys/time.h>
#include <time.h>

#define REDIS_HOST_BASE   "ares-stor-"
#define REDIS_HOST_START  25
#define REDIS_HOST_COUNT  8
#define REDIS_PORT_BASE   7000

#define MONGODB_HOST      "ares-comp-25-40g"
#define MONGODB_PORT      27017
#define MONGODB_DB_NAME   "integration"
#define MONGODB_COLL_NAME "symbios"

double timeval_substract(struct timeval *start, struct timeval *end);

#endif
