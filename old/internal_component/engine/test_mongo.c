#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <argp.h>
#include <stdbool.h>
#include "mpi.h"
#include "io_mongo.h"
#include "common.h"

extern mongoc_database_t *database;
extern mongoc_collection_t *collection;

/* Program documentation. */
static char doc[] =
"Synthetic Symbios engine evaluation";

/* A description of the arguments we accept. */
static char args_doc[] = ""; //-s req_size -p policy -t target_stor";

static struct argp_option options[] = {
  {"size", 's', "REQ_SIZE", 0, "Request size."},
  {"count", 'c', "COUNT", 0, "Number of operations."},
  {"policy", 'p', "POLICY", 0, "Policy."},
  {"target", 't', "TARGET", 0, "Target storage system."},
  {0}
};

struct arguments {
  int req_size;
  int count;
  char *policy;
  char *target;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = state->input;
  switch (key) {
    case 's': arguments->req_size = atoi(arg); break;
    case 'c': arguments->count = atoi(arg); break;
    case 'p': arguments->policy = arg; break;
    case 't': arguments->target = arg; break;
    /*case ARGP_KEY_NO_ARGS: argp_usage(state); break;*/
    /*case ARGP_KEY_ARG:*/
              /*if (state->arg_num >= 5) {*/
                /*argp_usage(state);*/
              /*}*/
              /*[>arguments->args[state->arg_num] = arg;<]*/
              /*break;*/
    /*case ARGP_KEY_END:*/
              /*printf("num of args: %d\n", state->arg_num);*/
              /*if (state->arg_num < 4) {*/
                /*argp_usage(state);*/
                /*[>exit(-1);<]*/
              /*}*/
              /*break;*/
    default: return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc};

// this mpi file test different insert size
int main(int argc, char *argv[])
{
  int i, req_size, total_size_per_rank, hostname_len, count;
  char key[1024], key_base[1024], hostname[1024];
  char *policy, *target;
  struct timespec start, end;
  char *buf;
  double duration, duration_max;
  bson_error_t error;
  int rank, nprocs;
  struct arguments arguments;

  arguments.req_size = -1;
  arguments.count = -1;
  arguments.policy = NULL;
  arguments.target = NULL;

  argp_parse(&argp, argc, argv, 0, 0, &arguments);

  req_size = arguments.req_size;
  count = arguments.count;
  policy = arguments.policy;
  target = arguments.target;
  total_size_per_rank = req_size * count;
  printf("Request size: %d\n", req_size);
  printf("Number of operations: %d\n", count);
  printf("Policy: %s\n", policy);
  printf("Target storage: %s\n", target);

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  /* Broadcast base file/key name */
  memset(key_base, 0, 1024);
  strcpy(key_base, KEY_BASE_STR);
  MPI_Bcast(key_base, 1024, MPI_BYTE, 0, MPI_COMM_WORLD);

  /* Connect to MongoDB server */
  MPI_Get_processor_name(hostname, &hostname_len);
  connectToMongoDBServerMPI(hostname);

  /* Allocate and init buffer */
  srand(rank + count + rank * count);
  if ((buf = (char *)malloc(req_size)) == NULL) {
    fprintf(stderr, "%s\n", "Allocate error!");
  }
  memset(buf, 'A', req_size);

  if (rank == 0)
    printf("Insert starts, "
           "each rank inserts %dx records in size of %d KiB ...\n",
            count, req_size / 1024);

  /* Insert data */
  clock_gettime(CLOCK_MONOTONIC, &start);
  MPI_Barrier(MPI_COMM_WORLD);
  for (i = 0; i < count; i++) {
    sprintf(key, "%s_%d_%d_%d", key_base, rank, rand(), i);
    // printf("%s\n", key);
    write_mongo_sized(collection, key, buf, req_size);
  }
  MPI_Barrier(MPI_COMM_WORLD);
  clock_gettime(CLOCK_MONOTONIC, &end);

  duration = timespec_substract(&start, &end);
  MPI_Reduce(&duration,
             &duration_max,
             1,
             MPI_DOUBLE,
             MPI_MAX,
             0,
             MPI_COMM_WORLD);
  if (rank == 0) printf("Insert finishes ...\n");

  /* Calculate time and throughput */
  if (rank == 0) {
    printf("Request size: %d B\n" , req_size);
    printf("Bandwidth: %lf MiB/s\n",
            (double) (total_size_per_rank / MEGA) * nprocs / duration_max);
    printf("Throughput of create_link (MongoDB): %lf op/s\n",
            count * nprocs / duration_max);
    printf("Total wall time: %lf s\n", duration_max);
  }

  /* Disconnect from MongoDB server */
  disconnectFromMongoDBServer();

  free(buf);
  MPI_Finalize();

  return EXIT_SUCCESS;
}
