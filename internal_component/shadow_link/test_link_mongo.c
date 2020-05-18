#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <argp.h>
#include <stdbool.h>
#include "mpi.h"
#include "io_mongo.h"
#include "common.h"

int rank, nprocs;

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
  {"mode", 'm', "MODE", 0, "0 for write, 1 for read."},
  {0}
};

struct arguments {
  int count;
  bool primary;
  bool shadow;
  char *target;
  int io_mode;
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
    case 'm':
      arguments->io_mode = atoi(arg); break;

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

void print_conf(struct arguments *conf)
{
  if (rank == 1) {
    printf("Number of operations: %d\n", conf->count);
    if (conf->io_mode == 0) {
      printf("I/O mode: write\n");
      if (conf->primary)
        printf("Metadata: primary\n");
      else if (conf->shadow) {
        printf("Metadata: shadow\n");
        printf("Target storage: %s\n", conf->target);
      }
    } else if (conf->io_mode == 1) {
      printf("I/O mode: read\n");
    }
  }
}

void result_report(char *test_name,
                   struct timespec *start,
                   struct timespec *end,
                   int count)
{
  double duration, duration_max;

  duration = timespec_substract(start, end);
  MPI_Reduce(&duration,
             &duration_max,
             1,
             MPI_DOUBLE,
             MPI_MAX,
             0,
             MPI_COMM_WORLD);
  if (rank == 0) printf("Test of %s finishes ...\n", test_name);

  /* Calculate time and throughput */
  if (rank == 0) {
    printf("Throughput of %s (MongoDB): %lf op/s\n",
            test_name, count * nprocs / duration_max);
    printf("Total wall time: %lf s\n", duration_max);
  }
}

void write_test(struct arguments *conf, char *key_base)
{
  int i;
  char key[1024];
  symbios_metadata metadata;

  for (i = 0; i < conf->count; i++) {
    sprintf(key, "%s_%d_%d_%d", key_base, rank, rand(), i);
    // printf("%s\n", key);
    metadata.primary = conf->primary;
    metadata.data_id = key;
    if (conf->primary == true) {
      metadata.target_stor = UNKNOWN;
      metadata.server = NULL;
      metadata.port = -1;
      metadata.offset = -1;
    } else {
      metadata.target_stor = PFS1;
      metadata.server = "ares-stor-15";
      metadata.port = 3334;
      metadata.offset = conf->count * 16384;
    }
    create_symbios_metadata_mongo(&metadata);
  }
}

void read_test(struct arguments *conf, char *key_base)
{
  int i;
  char key[1024];

  for (i = 0; i < conf->count; i++) {
    sprintf(key, "%s_%d_%d_%d", key_base, rank, rand(), i);
    // printf("%s\n", key);
    query_symbios_metadata_by_id_mongo(key);
  }
}

void do_test(struct arguments *conf, char *key_base)
{
  struct timespec start, end;
  double duration, duration_max;
  char *test_name;

  if (rank == 0) {
    printf("Test starts, "
           "each rank does %dx metadata operations ...\n", conf->count);
  }

  clock_gettime(CLOCK_MONOTONIC, &start);
  MPI_Barrier(MPI_COMM_WORLD);

  if (conf->io_mode == 0)
    write_test(conf, key_base);
  else if (conf->io_mode == 1)
    read_test(conf, key_base);

  MPI_Barrier(MPI_COMM_WORLD);
  clock_gettime(CLOCK_MONOTONIC, &end);

  if (conf->io_mode == 0) {
    if (conf->primary == true)
      test_name = "primary metadata creation";
    else
      test_name = "shadow metadata creation";
  } else
    test_name = "primary/shadow metadata query";
  result_report(test_name, &start, &end, conf->count);
}

// this mpi file test different insert size
int main(int argc, char *argv[])
{
  int hostname_len;
  char key_base[1024], hostname[1024];
  char *buf;
  bson_error_t error;
  struct arguments conf;

  /* Set default configuration */
  conf.count = -1;
  conf.primary = false;
  conf.shadow = false;
  conf.target = NULL;
  conf.io_mode = -1;

  /* Parse command line arguments */
  argp_parse(&argp, argc, argv, 0, 0, &conf);

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  /* Print input parameters */
  print_conf(&conf);

  /* Broadcast base file/key name */
  memset(key_base, 0, 1024);
  strcpy(key_base, KEY_BASE_STR);
  MPI_Bcast(key_base, 1024, MPI_BYTE, 0, MPI_COMM_WORLD);

  /* Connect to MongoDB server */
  MPI_Get_processor_name(hostname, &hostname_len);
  connectToMongoDBServerMPI(hostname);

  /* Set random seed */
  srand(rank + conf.count + rank * conf.count);

  /* Run tests */
  do_test(&conf, key_base);

  /* Disconnect from MongoDB server */
  disconnectFromMongoDBServer();

  MPI_Finalize();

  return EXIT_SUCCESS;
}
