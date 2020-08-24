#ifndef __LINK_FS_H__
#define __LINK_FS_H__

#include <sys/xattr.h>

int touch_link(char *filename);
int insert_link(char *filename, int seg_id, char *attr, char *target);
int remove_link(char *filename);

#endif
