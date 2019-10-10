#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mpi.h"
#include "hircluster.h"

#define NUM_OPS 1000
#define KILO            1024
#define MEGA            (1024 * 1024)

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
  char key[1024], temp[128];
  int i = 0, rank, nprocs;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s key\n", argv[0]);
    exit(-1);
  }

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  memset(key, 0, 1024);
  strcpy(key, argv[1]);
  MPI_Bcast(key, 1024, MPI_BYTE, 0, MPI_COMM_WORLD);

  redisClusterContext *cc;
  redisReply *reply;
  char nodelist[1024], key_str[1024];
  clock_t start, end;
  double duration;
  double duration_sum;

  /*strcpy(nodelist, "ares-comp-25-40g:7000,ares-comp-26-40g:7001,ares-comp-27-40g:7002");*/
  strcpy(nodelist, "ares-comp-25-40g:7000,ares-comp-26-40g:7001,ares-comp-27-40g:7002,ares-comp-28-40g:7003");
  /*strcpy(nodelist, "ares-comp-25-40g:7000,ares-comp-26-40g:7001,ares-comp-27-40g:7002,ares-comp-28-40g:7003,ares-comp-29-40g:7004,ares-comp-30-40g:7005,ares-comp-31-40g:7006,ares-comp-32-40g:7007");*/
  cc = redisClusterContextInit();
  /*printf("Node list: %s\n", nodelist);*/
  redisClusterSetOptionAddNodes(cc, nodelist);
  redisClusterConnect2(cc);
  if (cc != NULL && cc->err) {
    printf("Error: %s\n", cc->errstr);
    // handle error
  }

  /* Set keys using binary safe API */
  srand(1);
  char *buf = "OrangeFS: this is the actual file name";

  start = clock();
  while (i++ < NUM_OPS) {
    sprintf(temp, "_%d_%d", rank, i);
    strcpy(key_str, key);
    strcat(key_str, temp);
    /*printf("%s\n", key);*/
    reply = redisClusterCommand(cc, "SET %s %b", key, buf, strlen(key));
    if (strcmp(reply->str, "OK"))
      printf("Something is wrong\n");
    freeReplyObject(reply);
  }
  end = clock();
  duration = ((double) (end - start)) / CLOCKS_PER_SEC;

  MPI_Reduce(&duration, &duration_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  if (rank == 0) {
    printf("Throughput of shadow_creation (Redis): %lf op/s\n",
            NUM_OPS * nprocs * nprocs / duration_sum);
  }

  redisClusterReset(cc);

  /* Disconnects and frees the context */
  redisClusterFree(cc);

  MPI_Finalize();

  return 0;
}
