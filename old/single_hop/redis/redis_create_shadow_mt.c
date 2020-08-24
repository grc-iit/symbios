#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mpi.h"
#include "hircluster.h"
#include "link_redis.h"

#define NUM_OPS     1000
#define KILO        1024
#define MEGA        (1024 * 1024)

char *randstring(long length) {
  long n;
  static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,.-#'?!";
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

int main(int argc, char **argv)
{
  char key_base[1024], temp[1024];
  int i = 0, rank, nprocs;
  redisClusterContext *cc;
  clock_t start, end;
  double duration;
  double duration_sum;

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

  /* Connect to Redis server */
  cc = connectToRedisServer();

  srand(1);
  /* Create local links */
  start = clock();
  while (i++ < NUM_OPS) {
    sprintf(temp, "%s_%d_%d", key_base, rank, i);
    create_link_redis(cc, temp);
  }
  end = clock();
  duration = ((double) (end - start)) / CLOCKS_PER_SEC;

  /* Calculate time and throughput */
  MPI_Reduce(&duration, &duration_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  if (rank == 0) {
    printf("Throughput of create_link (Redis): %lf op/s\n",
            NUM_OPS * nprocs * nprocs / duration_sum);
  }

  /* Create return links */
  start = clock();
  i = 0;
  while (i++ < NUM_OPS) {
    sprintf(temp, "%s_%d_%d", key_base, rank, i + NUM_OPS);
    create_return_link_redis(cc, temp, "orangefs");
  }
  end = clock();
  duration = ((double) (end - start)) / CLOCKS_PER_SEC;

  /* Calculate time and throughput */
  MPI_Reduce(&duration, &duration_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  if (rank == 0) {
    printf("Throughput of create_return_link (Redis): %lf op/s\n",
            NUM_OPS * nprocs * nprocs / duration_sum);
  }
  /* Disconnect */
  disconnectFromRedisServer(cc);

  MPI_Finalize();

  return 0;
}
