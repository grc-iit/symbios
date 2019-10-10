#include "tlpi_hdr.h"
#include "common.h"

#define NUM_OPS 100

int main(int argc, char *argv[])
{
  char *value;
  int i = 0, ret;
  FILE *fd;
  struct timeval start, end;
  double duration = 0;

  if (argc < 2 || strcmp(argv[1], "--help") == 0)
    usageErr("%s file\n", argv[0]);

  while (i++ < NUM_OPS) {
    gettimeofday(&start, NULL);
    if ((fd = fopen(argv[1], "a+")) == NULL)
      errExit("fail to open %s\n", argv[1]);

    value = "The past is not dead.";
    if ((ret = fwrite(value, 1, strlen(value), fd)) != strlen(value))
      errExit("fail to write");

    if ((ret = fclose(fd)) != 0)
      errExit("fail to close");

    gettimeofday(&end, NULL);
    duration += timeval_substract(&start, &end);

    if ((ret = remove(argv[1])) == -1)
      errExit("fail to remove %s\n", argv[1]);
  }

  printf("Throughput of shadow_creation: %lf op/s\n", NUM_OPS / duration);

  exit(EXIT_SUCCESS);
}
