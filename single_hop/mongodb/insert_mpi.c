#include <mongoc/mongoc.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "mpi.h"
#include "link_mongo.h"

#define NUM_OPS     10

int main (int argc, char *argv[])
{
  int i, size, hostname_len;
  char key[1024], key_base[1024], hostname[1024];
  clock_t start, end;
  double duration, duration_sum;
  mongoc_client_t *client;
  bson_error_t error;
  int rank, nprocs;

  if (argc < 2) {
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

  /* Connect to MongoDB server */
  MPI_Get_processor_name(hostname, &hostname_len);
  connectToMongoDBServerMPI(hostname);

  srand(time(NULL) + rank);
  start = clock();
  for (i = 0; i < NUM_OPS; i++){
    sprintf(key, "%s_%d_%d", key_base, rank, rand());
    /*if (i == 0)*/
      /*printf("First key: %s\n", key);*/
    create_link_mongo(collection, key);
  }
  end = clock();
  duration = ((double) (end - start)) / CLOCKS_PER_SEC;

  /* Calculate time and throughput */
  MPI_Reduce(&duration, &duration_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  if (rank == 0) {
    printf("Throughput of create_link (MongoDB): %lf op/s\n",
            NUM_OPS * nprocs * nprocs / duration_sum);
  }

  /* Disconnect from MongoDB server */
  disconnectFromMongoDBServer();

  MPI_Finalize();

  return EXIT_SUCCESS;
}
