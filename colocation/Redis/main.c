#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <unistd.h>
#include "redis.h"

#define MEGA (1024 * 1024)

int main(int argc, char *argv[])
{
  int nprocs, rank, stop = 0, prepare_only = 0;
  char *trace_file;
  int time_limit;
  double start, end, duration;
  size_t total_size;
  double bandwidth;

  if (argc != 4) {
    printf("Usage: %s trace_file time_limit prepare_only\nExiting ...\n", argv[0]);
    exit(-1);
  }
  trace_file = argv[1];
  time_limit = atoi(argv[2]);
  prepare_only = atoi(argv[3]);

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  if (rank == 0) {
    printf("Reading trace from %s ...\n", trace_file);
    printf("Time limit: %d seconds\n", time_limit);
  }
  MPI_Bcast(&time_limit, 1, MPI_INT, 0, MPI_COMM_WORLD);

  redis_init(rank);
  prepare_data_redis(trace_file, rank);
  duration = 0;
  if (prepare_only == 0) {
    while (1) {
      start = MPI_Wtime();
      total_size = replay_trace_redis(trace_file, rank);
      end = MPI_Wtime();
      duration += end - start;
      if (duration > time_limit) break;
    }
  }
  if (rank == 0) {
    bandwidth = total_size / duration / MEGA * nprocs;
    printf("Redis total accessed size: %lf MB, time: %lf seconds, bandwidth: %lf MB/s\n",
            (double) total_size / MEGA * nprocs, duration, bandwidth);
  }
  redis_finalize(rank);

  MPI_Finalize();
  return 0;
}
