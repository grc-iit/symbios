#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "common.h"

double timespec_substract(struct timespec *start, struct timespec *end)
{
  if (start != NULL && end != NULL)
    return (end->tv_sec - start->tv_sec) + (end->tv_nsec - start->tv_nsec) * 1e-9;
  else
    return -1;
}

char *randstring(long length)
{
  long n;
  static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,.-#'?!";
  char *randomString = NULL;

  if (length) {
    randomString = (char *) malloc(sizeof(char) * (length + 1));
    if (randomString) {
      for (n = 0; n < length; n++) {
        int key = rand() % (int) (sizeof(charset) - 1);
        randomString[n] = charset[key];
      }
      randomString[length] = '\0';
    }
  }
  return randomString;
}

char *genstring(long length)
{
  char *string = NULL;

  if (length) {
    string = (char *) malloc(sizeof(char) * (length + 1));
    if (string)
      memset(string, 'A', sizeof(char) * (length + 1));
  }
  return string;
}
