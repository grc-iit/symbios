#ifndef __COMMON_H__
#define __COMMON_H__

#include <sys/time.h>
#include <time.h>

#define PFS1_STRIPE_SIZE  4096
#define PFS2_STRIPE_SIZE  131072
#define PFS_MAX_RETRY     3

#define REDIS_HOST_BASE   "ares-stor-"
#define REDIS_HOST_START  1
#define REDIS_HOST_COUNT  4
#define REDIS_PORT_BASE   7000

#define MONGODB_HOST      "ares-comp-25-40g"
#define MONGODB_PORT      27017
#define MONGODB_DB_NAME   "integration"
#define MONGODB_COLL_NAME "symbios"

#define TEST_SIZE         32768

#define KEY_BASE_STR      "test"
#define KILO              1024
#define MEGA              (1024 * 1024)
#define MAX_REQ_SIZE      (256 * MEGA)

#define DEBUG             1
#define debug_print(level, fmt, ...) \
        do { if (level <= DEBUG)\
          fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)

#define DECLARE_TIMING(s)  struct timespec timeStart_##s; struct timespec timeEnd_##s; double timeDiff_##s; double timeTally_##s = 0; int countTally_##s = 0
#define START_TIMING(s)    clock_gettime(CLOCK_MONOTONIC, &timeStart_##s)
#define STOP_TIMING(s)     clock_gettime(CLOCK_MONOTONIC, &timeEnd_##s); timeDiff_##s = (timeEnd_##s.tv_sec - timeStart_##s.tv_sec) + (timeEnd_##s.tv_nsec - timeStart_##s.tv_nsec) * 1e-9; timeTally_##s += timeDiff_##s; countTally_##s++
#define GET_TIMING(s)      timeTally_##s
#define GET_AVERAGE_TIMING(s)   (double)(countTally_##s ? timeTally_##s / countTally_##s : 0)
#define CLEAR_AVERAGE_TIMING(s) timeTally_##s = 0; countTally_##s = 0

double timespec_substract(struct timespec *start, struct timespec *end);
char *randstring(long length);
char *genstring(long length);

#endif
