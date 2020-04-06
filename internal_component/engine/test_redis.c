#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mpi.h"
#include "io_redis.h"
#include "common.h"

#define NUM_OPS     50000

int main(int argc, char **argv)
{
  char key_base[1024], temp[1024], *value;
  int i = 0, rank, nprocs;
  redisClusterContext *cc;
  struct timespec start, end;
  double duration;
  double duration_sum, duration_max, wall_clock_deviation;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s key_base\n", argv[0]);
    exit(-1);
  }

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  /* Broadcast base file/key name */
  memset(key_base, 0, 1024);
  strcpy(key_base, argv[1]);
  MPI_Bcast(key_base, 1024, MPI_BYTE, 0, MPI_COMM_WORLD);
  value = randstring(TEST_SIZE);

  /* Connect to Redis server */
  cc = connectToRedisServer();

  srand(1);
  /* Create local links */
  clock_gettime(CLOCK_MONOTONIC, &start);
  MPI_Barrier(MPI_COMM_WORLD);
  while (i++ < NUM_OPS) {
    sprintf(temp, "%s_%d_%d", key_base, rank, i);
    write_redis(cc, temp, value);
  }
  MPI_Barrier(MPI_COMM_WORLD);
  clock_gettime(CLOCK_MONOTONIC, &end);
  duration = timespec_substract(&start, &end);

  /* Calculate time and throughput */
  MPI_Reduce(&duration,
             &duration_max,
             1,
             MPI_DOUBLE,
             MPI_MAX,
             0,
             MPI_COMM_WORLD);
  if (rank == 0) {
    printf("Duration (max): %lf seconds\n", duration_max);
    printf("Write throughput (Redis): %lf op/s\n",
            NUM_OPS * nprocs / duration_max);
    printf("Write bandwidth (Redis): %lf MB/s\n",
            NUM_OPS * TEST_SIZE / 1024 * nprocs / 1024 / duration_max);
  }

  free(value);

  /* Disconnect */
  disconnectFromRedisServer(cc);

  MPI_Finalize();

  return 0;
}
