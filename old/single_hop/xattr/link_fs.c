#include <stdio.h>
#include <string.h>
#include "tlpi_hdr.h"
#include "link_fs.h"
#include "common.h"

int touch_link(char *filename)
{
  FILE *fd;
  if ((fd = fopen(filename, "a+")) == NULL)
    errExit("fail to open %s\n", filename);
  return fclose(fd);
}

int insert_link(char *filename, int seg_id, char *attr, char *target)
{
  char key[1024];
  sprintf(key, "%s:%s_%d", target, filename, seg_id);
  if (setxattr(filename, attr, key, strlen(key), 0) == -1)
    errExit("setxattr");
  else
    return 0;
}

int remove_link(char *filename)
{
  int ret;
  if ((ret = remove(filename)) == -1)
    errExit("fail to remove %s\n", filename);
  else
    return ret;
}
