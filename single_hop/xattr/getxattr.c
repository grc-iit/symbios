#include <sys/xattr.h>
#include "tlpi_hdr.h"
#include "common.h"

#define NUM_OPS 10000
#define XATTR_SIZE 10000

static void usageError(char *progName)
{
  fprintf(stderr, "Usage: %s [-x] file...\n", progName);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
  char list[XATTR_SIZE], value[XATTR_SIZE];
  ssize_t listLen, valueLen;
  int ns, j, k, opt;
  Boolean hexDisplay;
  int i = 0;
  struct timeval start, end;

  hexDisplay = 0;
  while ((opt = getopt(argc, argv, "x")) != -1) {
    switch (opt) {
      case 'x': hexDisplay = 1;       break;
      case '?': usageError(argv[0]);
    }
  }

  if (optind >= argc)
    usageError(argv[0]);

  gettimeofday(&start, NULL);
  while (i++ < NUM_OPS) {
    for (j = optind; j < argc; j++) {
      listLen = listxattr(argv[j], list, XATTR_SIZE);
      if (listLen == -1)
        errExit("listxattr");

      /*printf("%s:\n", argv[j]);*/

      /* Loop through all EA names, displaying name + value */
      for (ns = 0; ns < listLen; ns += strlen(&list[ns]) + 1) {
        valueLen = getxattr(argv[j], &list[ns], value, XATTR_SIZE);
      }
    }
  }
  gettimeofday(&end, NULL);
  printf("Throughput of getxattr: %lf op/s\n", NUM_OPS / timeval_substract(&start, &end));

  exit(EXIT_SUCCESS);
}
