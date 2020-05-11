#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <argp.h>
#include <stdbool.h>
#include "mpi.h"
#include "io_mongo.h"
#include "common.h"

/* Program documentation. */
static char doc[] =
"Synthetic Symbios metadata evaluation (MongoDB)";

/* A description of the arguments we accept. */
static char args_doc[] = "";

static struct argp_option options[] = {
  {"count", 'c', "COUNT", 0, "Number of operations."},
  {"primary", 'p', NULL, false, "Primary copy of metadata."},
  {"shadow", 's', NULL, false, "Shadow copy of metadata."},
  {"target", 't', "TARGET", 0, "Target storage system in shadow copy of metadata."},
  {0}
};

struct arguments {
  int count;
  bool primary;
  bool shadow;
  char *target;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = state->input;
  switch (key) {
    case 'c':
      arguments->count = atoi(arg); break;
    case 'p':
      if (arguments->shadow == true) {
        fprintf(stderr, "Metadata can only be either primary or shadow\n");
        argp_usage(state);
      } else arguments->primary = true; break;
    case 's':
      if (arguments->primary == true) {
        fprintf(stderr, "Metadata can only be either primary or shadow\n");
        argp_usage(state);
      } else arguments->shadow = true; break;
    case 't':
      arguments->target = arg; break;

    /*case ARGP_KEY_NO_ARGS:*/
      /*printf("no argument\n");*/
      /*argp_usage(state);*/

    /*case ARGP_KEY_ARG:*/
      /*if (state->arg_num >= 5) {*/
        /*printf("too many arguments\n");*/
        /*argp_usage(state);*/
      /*}*/
      /*arguments->args[state->arg_num] = arg;*/
      /*break;*/
    /*case ARGP_KEY_END:*/
      /*printf("num of args: %d\n", state->arg_num);*/
      /*if (state->arg_num < 4) {*/
        /*argp_usage(state);*/
        /*exit(-1);*/
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
  int i, hostname_len, count;
  char key[1024], key_base[1024], hostname[1024];
  bool primary, shadow;
  char *target;
  struct timespec start, end;
  char *buf;
  double duration, duration_max;
  bson_error_t error;
  int rank, nprocs;
  struct arguments arguments;
  symbios_metadata metadata;

  arguments.count = -1;
  arguments.primary = false;
  arguments.shadow = false;
  arguments.target = NULL;

  argp_parse(&argp, argc, argv, 0, 0, &arguments);

  count = arguments.count;
  primary = arguments.primary;
  shadow = arguments.shadow;
  target = arguments.target;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  if (rank == 1) {
    printf("Number of operations: %d\n", count);
    if (primary)
      printf("Metadata: primary\n");
    else if (shadow) {
      printf("Metadata: shadow\n");
      printf("Target storage: %s\n", target);
    }
  }

  /* Broadcast base file/key name */
  memset(key_base, 0, 1024);
  strcpy(key_base, KEY_BASE_STR);
  MPI_Bcast(key_base, 1024, MPI_BYTE, 0, MPI_COMM_WORLD);

  /* Connect to MongoDB server */
  MPI_Get_processor_name(hostname, &hostname_len);
  connectToMongoDBServerMPI(hostname);

  /* Set random seed */
  srand(rank + count + rank * count);

  if (primary) {
    if (rank == 0)
      printf("Test starts, "
             "each rank creates %dx copies of primary metadata ...\n", count);

    /* Create primary metadata */
    clock_gettime(CLOCK_MONOTONIC, &start);
    MPI_Barrier(MPI_COMM_WORLD);
    for (i = 0; i < count; i++) {
      sprintf(key, "%s_%d_%d_%d", key_base, rank, rand(), i);
      // printf("%s\n", key);
      metadata.primary = true;
      metadata.data_id = key;
      metadata.target_stor = UNKNOWN;
      metadata.server = NULL;
      metadata.port = -1;
      metadata.offset = -1;
      create_symbios_metadata_mongo(&metadata);
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
    if (rank == 0) printf("Test of primary metadata finishes ...\n");

    /* Calculate time and throughput */
    if (rank == 0) {
      printf("Throughput of primary meda creation (MongoDB): %lf op/s\n",
              count * nprocs / duration_max);
      printf("Total wall time: %lf s\n", duration_max);
    }
  } else {
    if (rank == 0)
      printf("Test starts, "
             "each rank creates %dx copies of shadow metadata "
             "poining to %s ...\n",
              count, target);

    /* Create shadow metadata */
    clock_gettime(CLOCK_MONOTONIC, &start);
    MPI_Barrier(MPI_COMM_WORLD);
    for (i = 0; i < count; i++) {
      sprintf(key, "%s_%d_%d_%d", key_base, rank, rand(), i);
      // printf("%s\n", key);
      metadata.primary = false;
      metadata.data_id = key;
      metadata.target_stor = PFS1;
      metadata.server = "ares-stor-15";
      metadata.port = 3334;
      metadata.offset = count * 16384;
      create_symbios_metadata_mongo(&metadata);
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
    if (rank == 0) printf("Test of shadow metadata finishes ...\n");

    /* Calculate time and throughput */
    if (rank == 0) {
      printf("Throughput of shadow meda creation (MongoDB): %lf op/s\n",
              count * nprocs / duration_max);
      printf("Total wall time: %lf s\n", duration_max);
    }
  }
  /* Disconnect from MongoDB server */
  disconnectFromMongoDBServer();

  MPI_Finalize();

  return EXIT_SUCCESS;
}
