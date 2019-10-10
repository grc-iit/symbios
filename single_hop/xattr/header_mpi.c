#include <sys/xattr.h>
#include "mpi.h"
#include "tlpi_hdr.h"
#include "common.h"

#define NUM_OPS 100

int main(int argc, char *argv[])
{
  char *value, filename[1024];
  int i = 0, ret, nprocs, rank;
  MPI_File fh;
  struct timeval start, end;
  double duration = 0, duration_sum;

  if (argc < 2 || strcmp(argv[1], "--help") == 0)
    usageErr("%s file\n", argv[0]);

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  memset(filename, 0, 1024);
  if (nprocs == 1)
    strcpy(filename, argv[1]);
  else
    sprintf(filename, "%s_%d", argv[1], rank);

  while (i++ < NUM_OPS) {
    gettimeofday(&start, NULL);
    if ((ret = MPI_File_open(MPI_COMM_SELF,
                             filename,
                             MPI_MODE_CREATE | MPI_MODE_RDWR,
                             MPI_INFO_NULL,
                             &fh)) != 0)
      errExit("fail to open %s\n", filename);

    value = "The past is not dead.";
    if ((ret = MPI_File_write(fh,
                              value,
                              strlen(value),
                              MPI_BYTE,
                              MPI_STATUS_IGNORE)) != 0)
      errExit("fail to write");

    if ((ret = MPI_File_close(&fh)) != 0)
      errExit("fail to close");

    gettimeofday(&end, NULL);
    duration += timeval_substract(&start, &end);

    if ((ret = MPI_File_delete(filename, MPI_INFO_NULL)) != 0)
      errExit("fail to delete");
  }

  if (nprocs == 1)
    printf("Throughput of shadow_creation (header): %lf op/s\n", NUM_OPS / duration);
  else {
    MPI_Reduce(&duration, &duration_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    if (rank == 0) {
      printf("Throughput of shadow_creation (header): %lf op/s\n",
              NUM_OPS * nprocs * nprocs / duration_sum);
    }
  }

  MPI_Finalize();

  exit(EXIT_SUCCESS);
}
