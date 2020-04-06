#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <argp.h>
#include <stdbool.h>
#include "mpi.h"
#include "io_mongo.h"
#include "io_redis.h"
#include "common.h"

#define PFS1_DEST_FILE "/mnt/nvme/kfeng/pvfs2-mount/engine.dat"
#define PFS2_DEST_FILE "/mnt/nvme/kfeng/pvfs2-mount2/engine.dat"

extern mongoc_collection_t *collection;
extern mongoc_database_t *database;

/**
 * Global variables
 */
FILE *dest_fp1, *dest_fp2;
redisClusterContext *redis_cc;
char *buf;
long total_size_per_rank;
int nprocs;
int rank;

DECLARE_TIMING(1);

/*
 * Command line argument parsing
 */
/* Program documentation. */
static char doc[] =
"Synthetic Symbios engine evaluation";

/* A description of the arguments we accept. */
static char args_doc[] = "-s req_size -p policy -t target_stor -f trace_file";

static struct argp_option options[] = {
  {"size", 's', "REQ_SIZE", 0, "Request size."},
  {"count", 'c', "COUNT", 0, "Number of operations."},
  {"policy", 'p', "POLICY", 0, "Policy."},
  {"target", 't', "TARGET", 0, "Target storage system."},
  {"trace_file", 'f', "TRACE_FILE", 0, "Trace file to replay."},
  {"redis_hosts", 'r', "REDIS_HOSTFILE", 0, "Redis hostfile."},
  {0}
};

struct arguments {
  int req_size;
  int count;
  char *policy;
  char *target;
  char *trace_file;
  char *redis_hosts;
  /*char *mongo_hosts;*/
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = state->input;
  switch (key) {
    case 's': arguments->req_size = atoi(arg); break;
    case 'c': arguments->count = atoi(arg); break;
    case 'p': arguments->policy = arg; break;
    case 't': arguments->target = arg; break;
    case 'f': arguments->trace_file = arg; break;
    case 'r': arguments->redis_hosts = arg; break;
    /*case 'm': arguments->mongo_hosts = arg; break;*/
    case ARGP_KEY_ARG:
              if (state->arg_num >= 8) {
                argp_usage(state);
                MPI_Abort(MPI_COMM_WORLD, 200);
              }
              break;
    case ARGP_KEY_END:
              if (arguments->trace_file == NULL ||
                  arguments->redis_hosts == NULL) {
                argp_usage(state);
                MPI_Abort(MPI_COMM_WORLD, 200);
              }
              break;
    default: return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc};

/*
 * Policy-related
 */
enum target {PFS1 = 0,
             PFS2 = 1,
             REDIS = 2,
             MONGO = 3};

char target_stor_list[][32] = {"PFS1", "PFS2", "REDIS", "MONGO"};

enum target policy_random()
{
  enum target target_stor;
  return (target_stor = (rand() % sizeof(target_stor_list)));
}

enum target policy_roundrobin()
{
  enum target target_stor;
  return (target_stor = (rand() % sizeof(target_stor_list)));
}

enum target policy_heuristic()
{
  return PFS1;
}

enum target policy_model()
{
  return PFS1;
}

int select_target(char *policy)
{
  enum target target_stor;

  if (strcmp(policy, "RANDOM") == 0) {
    target_stor = policy_random();
  } else if (strcmp(policy, "ROUND-ROBIN") == 0) {
    target_stor = policy_roundrobin();
  } else if (strcmp(policy, "HEURISTIC") == 0) {
    target_stor = policy_heuristic();
  } else if (strcmp(policy, "MODEL") == 0) {
    target_stor = policy_model();
  }
  debug_print(1, "Target is set to %d\n", target_stor);

  return target_stor;
}

/*
 * Trace replayer
 */
enum io_ops {OPEN = 0,
             READ = 1,
             WRITE = 2,
             SEEK = 3,
             CLOSE = 4};

typedef struct trace_record_ {
  enum io_ops ops;
  size_t size;
  long offset;
} trace_record;

int get_record_count(FILE *trace_fp)
{
  char c;
  int record_cnt = 0;

  for (c = getc(trace_fp); c != EOF; c = getc(trace_fp))
    if (c == '\n')
      record_cnt++;
  rewind(trace_fp);
  return record_cnt;
}

trace_record *next_trace_record(FILE *trace_fp, trace_record *record, int rank)
{
  char line[1024], *token;

  memset(line, 0, 1024);
  if (rank == 0 && fgets(line, 1024, trace_fp) == NULL) {
    debug_print(1, "End of trace file reached\n");
    free(record);
    record = NULL;
    return NULL;
  }
  MPI_Bcast(line, 1024, MPI_BYTE, 0, MPI_COMM_WORLD);
  debug_print(2, "trace record: %s", line);

  token = strtok(line, ",");
  if ((strstr(token, "OPEN") != NULL) ||
      (strstr(token, "open") != NULL)) {
    record->ops = OPEN;
  } else if ((strstr(token, "READ") != NULL) ||
            (strstr(token, "read") != NULL)) {
    record->ops = READ;
  } else if ((strstr(token, "WRITE") != NULL) ||
            (strstr(token, "write") != NULL)) {
    record->ops = WRITE;
  } else if ((strstr(token, "SEEK") != NULL) ||
            (strstr(token, "seek") != NULL)) {
    record->ops = SEEK;
  } else if ((strstr(token, "CLOSE") != NULL) ||
            (strstr(token, "close") != NULL)) {
    record->ops = CLOSE;
  }
  token = strtok(NULL, ",");
  record->offset = atoi(token);
  token = strtok(NULL, ",");
  record->size = atoi(token);
  total_size_per_rank += (long) record->size;

  return record;
}

/*
 * I/O-related
 */
void pfs_io(int stripe_size, trace_record *record)
{
  FILE *dest_fp;

  if (stripe_size == PFS1_STRIPE_SIZE)
    dest_fp = fdopen(dup(fileno(dest_fp1)), "w");
  else if (stripe_size == PFS2_STRIPE_SIZE)
    dest_fp = fdopen(dup(fileno(dest_fp2)), "w");

  if (record->ops == WRITE) {
    if (fwrite(buf, record->size, 1, dest_fp) < 0) {
      fprintf(stderr, "Fail to write to destination file\n");
      exit(-1);
    } else
      debug_print(2, "Write %d bytes to PFS\n", record->size);
  } else if (record->ops == READ) {
    if (fread(buf, record->size, 1, dest_fp) < 0) {
      fprintf(stderr, "Fail to read from destination file\n");
      exit(-1);
    } else
      debug_print(2, "Read %d bytes from PFS\n", record->size);
  }
}

void redis_io(trace_record *record)
{
  char key[1024];

  sprintf(key, "%s_%d_%d", KEY_BASE_STR, rank, rand());
  if (record->ops == WRITE) {
    write_redis_sized(redis_cc, key, buf, record->size);
  } else if (record->ops == READ) {
    read_redis_sized(redis_cc, key, buf, record->size);
  }
}

void mongo_io(trace_record *record)
{
  char key[1024];

  sprintf(key, "%s_%d_%d", KEY_BASE_STR, rank, rand());
  if (record->ops == WRITE) {
    write_mongo_sized(collection, key, buf, record->size);
  } else if (record->ops == READ) {
    read_mongo_sized(collection, key, buf, record->size);
  }
}

void do_io(char *target, char *trace_file, int rank)
{
  FILE *trace_fp;
  trace_record *record;
  size_t size;
  int i, record_cnt, finished_record_cnt;

  if (rank == 0) {
    if ((trace_fp = fopen(trace_file, "r")) == NULL) {
      fprintf(stderr, "Fail to open trace file %s\n", trace_file);
      MPI_Abort(MPI_COMM_WORLD, 205);
    }
    record_cnt = get_record_count(trace_fp);
  }
  MPI_Bcast(&record_cnt, 1, MPI_INT, 0, MPI_COMM_WORLD);
  printf("Number of record: %d\n", record_cnt);

  record = (trace_record *) malloc(sizeof(trace_record));

  finished_record_cnt = 0;
  while (finished_record_cnt++ < record_cnt - 1) {
    record = next_trace_record(trace_fp, record, rank);

    START_TIMING(1);
    /*debug_print(1, "TRACE: op: %d, size: %d\n", record->ops, record->size);*/
    if (strcmp(target, "PFS1") == 0) {
      pfs_io(PFS1_STRIPE_SIZE, record);
    } else if (strcmp(target, "PFS2") == 0) {
      pfs_io(PFS2_STRIPE_SIZE, record);
    } else if (strcmp(target, "REDIS") == 0) {
      redis_io(record);
    } else if (strcmp(target, "MONGO") == 0) {
      mongo_io(record);
    }
    STOP_TIMING(1);
  }

  if (rank == 0 && trace_fp) fclose(trace_fp);
}

/**
 * Initilization and finalization
 */
void init(char *target_stor, char *redis_hosts, int max_req_size)
{
  char hostname[1024];
  char pfs1_dest_file_name[PATH_MAX], pfs2_dest_file_name[PATH_MAX];
  int hostname_len, retries;

  debug_print(1, "In init(), target stor: %s\n", target_stor);
  // Open file on PFS1
  if (strcmp(target_stor, "") == 0 || strcmp(target_stor, "PFS1") == 0) {
    retries = 0;
    sprintf(pfs1_dest_file_name, "%s_%d", PFS1_DEST_FILE, rank);
    while ((dest_fp1 = fopen(pfs1_dest_file_name, "w+")) == NULL &&
           retries++ < PFS_MAX_RETRY) {
      debug_print(1, "Retry opening dest file %s on PFS1\n",
                  pfs1_dest_file_name);
    }
    if (dest_fp1 == NULL) {
      fprintf(stderr, "Fail to open dest file %s on PFS1\n",
              pfs1_dest_file_name);
      MPI_Abort(MPI_COMM_WORLD, 201);
    } else
      debug_print(1, "Dest file %s on PFS1 is opened\n",
                  pfs1_dest_file_name);
  }

  // Open file on PFS2
  if (strcmp(target_stor, "") == 0 || strcmp(target_stor, "PFS2") == 0) {
    retries = 0;
    sprintf(pfs2_dest_file_name, "%s_%d", PFS2_DEST_FILE, rank);
    while ((dest_fp2 = fopen(pfs2_dest_file_name, "w+")) == NULL &&
           retries++ < PFS_MAX_RETRY) {
      debug_print(1, "Retry opening dest file %s on PFS2\n",
                  pfs2_dest_file_name);
    }
    if (dest_fp2 == NULL) {
      fprintf(stderr, "Fail to open dest file %s on PFS2\n",
              pfs2_dest_file_name);
      MPI_Abort(MPI_COMM_WORLD, 202);
    } else
      debug_print(1, "Dest file %s on PFS2 is opened\n",
                  pfs2_dest_file_name);
  }

  // Connect to Redis cluster
  if (strcmp(target_stor, "") == 0 || strcmp(target_stor, "REDIS") == 0) {
    if ((redis_cc = connectToRedisServerWithHostfile(redis_hosts)) == NULL) {
      fprintf(stderr, "Fail to connect to Redis server\n");
      MPI_Abort(MPI_COMM_WORLD, 203);
    }
    debug_print(1, "Redis cluster is connected\n");
  }

  // Connect to MongoDB
  if (strcmp(target_stor, "") == 0 || strcmp(target_stor, "MONGO") == 0) {
    MPI_Get_processor_name(hostname, &hostname_len);
    if (connectToMongoDBServerMPI(hostname) == EXIT_FAILURE) {
      fprintf(stderr, "Fail to connect to MongoDB server\n");
      MPI_Abort(MPI_COMM_WORLD, 204);
    }
    debug_print(1, "MongoDB is connected\n");
  }

  // Prepare write buffer
  srand(rank);
  buf = genstring(max_req_size);
}

void finalize()
{
  if (dest_fp1 != NULL)
    fclose(dest_fp1);
  if (dest_fp2 != NULL)
    fclose(dest_fp2);
  if (redis_cc != NULL)
    disconnectFromRedisServer(redis_cc);
  if (database != NULL && collection != NULL)
    disconnectFromMongoDBServer();
  free(buf);
}

int main(int argc, char *argv[])
{
  struct arguments arguments;
  int req_size, max_req_size, count;
  char policy[128], target[128], trace_file[128], redis_hosts[128];
  double duration, duration_max;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  if (rank == 0) {
    arguments.req_size = -1;
    arguments.count = -1;
    arguments.policy = "RANDOM";
    arguments.target = "PFS1";
    arguments.redis_hosts = NULL;

    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    req_size = arguments.req_size;
    count = arguments.count;
    strcpy(policy, arguments.policy);
    strcpy(target, arguments.target);
    realpath(arguments.trace_file, trace_file);
    realpath(arguments.redis_hosts, redis_hosts);
    total_size_per_rank = 0;

    printf("Request size: %d\n", req_size);
    printf("Number of operations: %d\n", count);
    printf("Policy: %s\n", policy);
    printf("Target storage: %s\n", target);
    printf("Trace file: %s\n", trace_file);
    printf("Redis hostfile: %s\n", redis_hosts);
  }

  MPI_Bcast(&req_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&count, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(policy, 128, MPI_BYTE, 0, MPI_COMM_WORLD);
  MPI_Bcast(target, 128, MPI_BYTE, 0, MPI_COMM_WORLD);
  MPI_Bcast(trace_file, 128, MPI_BYTE, 0, MPI_COMM_WORLD);
  MPI_Bcast(redis_hosts, 128, MPI_BYTE, 0, MPI_COMM_WORLD);

  if (strcmp(target, "") == 0)
    strcpy(target, target_stor_list[select_target(policy)]);

  if (rank == 0) printf("Initializing ...\n");
  if (req_size < 0)
    max_req_size = MAX_REQ_SIZE;
  else
    max_req_size = req_size;
  init(target, redis_hosts, max_req_size);

  if (rank == 0) printf("Starting I/O test ...\n");
  do_io(target, trace_file, rank);
  duration = GET_TIMING(1);
  MPI_Reduce(&duration,
             &duration_max,
             1,
             MPI_DOUBLE,
             MPI_MAX,
             0,
             MPI_COMM_WORLD);
  if (rank == 0) {
    printf("Duration: %lf seconds\n", duration_max);
    printf("Bandwidth: %lf MiB/s\n",
            total_size_per_rank / duration_max * nprocs / MEGA);
  }

  if (rank == 0) printf("Finalizing ...\n");
  finalize();

  MPI_Finalize();
  return 0;
}
