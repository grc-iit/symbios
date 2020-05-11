#include <stdio.h>
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

/*
 * Serializer and deserializer
 */
char *serialize_metadata(symbios_metadata *metadata)
{
  char *metadata_str;
  char temp[128];

  metadata_str = (char *) malloc(1024);

  // data_id
  memset(metadata_str, 0, 1024);
  strcpy(metadata_str, metadata->data_id);
  strcat(metadata_str, SERDE_MD_SEP);
  debug_print(2, "data_id serialized\n");

  // primary flag
  if (metadata->primary == true)
    strcat(metadata_str, "1");
  else
    strcat(metadata_str, "0");
  strcat(metadata_str, SERDE_MD_SEP);
  debug_print(2, "primary serialized\n");

  // target storage
  memset(temp, 0, 128);
  sprintf(temp, "%d", metadata->target_stor);
  strcat(metadata_str, temp);
  strcat(metadata_str, SERDE_MD_SEP);
  debug_print(2, "target_stor serialized\n");

  // server
  if (metadata->server != NULL)
    strcat(metadata_str, metadata->server);
  strcat(metadata_str, SERDE_MD_SEP);
  debug_print(2, "server serialized\n");

  // port
  memset(temp, 0, 128);
  sprintf(temp, "%d", metadata->port);
  strcat(metadata_str, temp);
  debug_print(2, "port serialized\n");

  // offset
  memset(temp, 0, 128);
  sprintf(temp, "%d", metadata->offset);
  strcat(metadata_str, temp);
  strcat(metadata_str, SERDE_MD_SEP);
  debug_print(2, "offset serialized\n");

  debug_print(1, "metadata str: %s\n", metadata_str);
  return metadata_str;
}

symbios_metadata *deserialize_metadata(char *metadata_str)
{
  symbios_metadata *metadata;
  char *token;

  metadata = (symbios_metadata *) malloc(sizeof(symbios_metadata));

  // data_id
  token = strtok(metadata_str, SERDE_MD_SEP);
  metadata->data_id = token;
  debug_print(1, "data_id: %s\n", metadata->data_id);

  // primary flag
  token = strtok(metadata_str, SERDE_MD_SEP);
  metadata->primary = atoi(token);
  debug_print(1, "primary: %d\n", metadata->primary);

  // target storage
  token = strtok(metadata_str, SERDE_MD_SEP);
  metadata->primary = atoi(token);
  debug_print(1, "primary: %d\n", metadata->primary);

  // server
  token = strtok(metadata_str, SERDE_MD_SEP);
  metadata->server = token;
  debug_print(1, "server: %s\n", metadata->server);

  // port
  token = strtok(metadata_str, SERDE_MD_SEP);
  metadata->port = atoi(token);
  debug_print(1, "port: %d\n", metadata->port);

  // offset
  token = strtok(metadata_str, SERDE_MD_SEP);
  metadata->offset = atoi(token);
  debug_print(1, "offset: %d\n", metadata->offset);

  return metadata;
}
