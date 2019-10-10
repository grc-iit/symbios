#include <sys/xattr.h>
#include "mpi.h"
#include "tlpi_hdr.h"
#include "common.h"

#define NUM_OPS 10

int main(int argc, char *argv[])
{
  char *value, filename[1024];
  int i = 0, ret, nprocs, rank;
  FILE *fd;
  struct timeval start, end;
  double duration = 0, duration_sum;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  memset(filename, 0, 1024);
  if (nprocs == 1)
    strcpy(filename, argv[1]);
  else
    sprintf(filename, "%s_%d", argv[1], rank);
  if (argc < 2 || strcmp(argv[1], "--help") == 0)
    usageErr("%s file\n", argv[0]);
  /*printf("%s\n", filename);*/

  while (i++ < NUM_OPS) {
    gettimeofday(&start, NULL);
    if ((fd = fopen(filename, "a+")) == NULL)
      errExit("fail to open %s\n", filename);
    fclose(fd);
    /*printf("aaa\n");*/

    value = "The past is not dead.";
    if (setxattr(filename, "user.x", value, strlen(value), 0) == -1)
      errExit("setxattr");
    /*printf("aaa\n");*/

    gettimeofday(&end, NULL);
    duration += timeval_substract(&start, &end);

    if ((ret = remove(filename)) == -1)
      errExit("fail to remove %s\n", filename);
    /*printf("aaa\n");*/
  }

  if (nprocs == 1)
    printf("Throughput of shadow_creation (setxattr): %lf op/s\n", NUM_OPS / duration);
  else {
    MPI_Reduce(&duration, &duration_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    if (rank == 0) {
      printf("Throughput of shadow_creation (setxattr): %lf op/s\n",
              NUM_OPS * nprocs * nprocs / duration_sum);
    }
  }

  MPI_Finalize();

  exit(EXIT_SUCCESS);
}
