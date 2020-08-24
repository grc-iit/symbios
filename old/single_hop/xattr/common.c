#include <sys/time.h>
#include <time.h>
#include "common.h"

double timeval_substract(struct timeval *start, struct timeval *end)
{
  if (start != NULL && end != NULL)
    return (end->tv_sec - start->tv_sec) + (end->tv_usec - start->tv_usec) / 1000000.0;
  else
    return -1;
}

