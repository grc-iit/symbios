#include <mongoc/mongoc.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "link_mongo.h"

#define NUM_OPS     1000

int main (int argc, char *argv[])
{
  char *uri_string; //= "mongodb://172.25.101.1:27017";
  char *buf;
  int i, size;
  char key[128];
  clock_t start, end;
  double duration;
  mongoc_uri_t *uri;
  mongoc_client_t *client;

  if (argc < 2) {
    fprintf(stderr, "Usage: %s size\n", argv[0]);
    exit(-1);
  }
  size = atoi(argv[1]);
  if ((buf = (char *)malloc(size)) == NULL) {
    fprintf(stderr, "%s\n", "Allocate error!");
  }
  // allocate 'A' into the whole space
  memset(buf, 'A', size);

  /* Connect to MongoDB server */
  connectToMongoDBServer();

  start = clock();
  for (i = 0; i < NUM_OPS; i++){
    sprintf(key, "key_%d", i);
    create_link_mongo_sized(collection, key, buf);
  }
  end = clock();
  duration = ((double) (end - start)) / CLOCKS_PER_SEC;

  printf("Throughput of create_link (MongoDB): %lf op/s\n",
          NUM_OPS / duration);

  /* Disconnect from MongoDB server */
  disconnectFromMongoDBServer();

  return EXIT_SUCCESS;
}
