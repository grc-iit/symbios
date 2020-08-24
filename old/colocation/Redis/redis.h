//
// Created by kfeng on 4/18/19.
//

#ifndef REDIS_REDIS_H
#define REDIS_REDIS_H

#include <hircluster.h>

#define MAX_LINE_LEN      128
#define MAX_KEY_LEN       128
#define MAX_NODELIST_LEN  512

char *randstring(long length);

int redis_init(int rank);
int prepare_data_redis(char *trace_file, int rank);
size_t replay_trace_redis(char *trace_file, int rank);
void redis_finalize(int rank);

#endif //REDIS_REDIS_H
