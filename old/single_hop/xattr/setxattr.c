#include <sys/time.h>
#include "mpi.h"
#include "tlpi_hdr.h"
#include "common.h"
#include "link_fs.h"

#define NUM_OPS 100

int main(int argc, char *argv[])
{
  char *value, filename[1024];
  int i = 0, ret, nprocs, rank;
  struct timeval start1, start2, end;
  double duration1 = 0, duration2 = 0, duration_sum1, duration_sum2;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (argc < 2 || strcmp(argv[1], "--help") == 0)
    usageErr("%s file\n", argv[0]);

  while (i++ < NUM_OPS) {
    memset(filename, 0, 1024);
    if (nprocs == 1)
      sprintf(filename, "%s_%d", argv[1], i);
    else
      sprintf(filename, "%s_%d_%d", argv[1], rank, i);

    gettimeofday(&start1, NULL);
    touch_link(filename);

    gettimeofday(&start2, NULL);
    insert_link(filename, i, "loc", "redis");

    gettimeofday(&end, NULL);
    duration1 += timeval_substract(&start1, &end);
    duration2 += timeval_substract(&start2, &end);

    remove_link(filename);
  }

  if (nprocs == 1) {
    printf("Throughput of shadow_creation (setxattr): %lf op/s\n", NUM_OPS / duration1);
    printf("Throughput of shadow_insert (setxattr): %lf op/s\n", NUM_OPS / duration2);
  } else {
    MPI_Reduce(&duration1, &duration_sum1, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&duration2, &duration_sum2, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    if (rank == 0) {
      printf("Throughput of shadow_creation (setxattr): %lf op/s\n",
              NUM_OPS * nprocs * nprocs / duration_sum1);
      printf("Throughput of shadow_insert (setxattr): %lf op/s\n",
              NUM_OPS * nprocs * nprocs / duration_sum2);
    }
  }

  MPI_Finalize();

  exit(EXIT_SUCCESS);
}
