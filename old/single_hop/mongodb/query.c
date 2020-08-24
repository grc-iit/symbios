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
  mongoc_client_t *client;
  mongoc_cursor_t *cursor;
  const bson_t *doc;
  bson_t *query;
  char *query_str;

  /*
   * Command line argument parsing
   */
  if (argc < 3) {
    fprintf(stderr, "Usage: %s URI size\n", argv[0]);
    exit(-1);
  }
  uri_string = argv[1];
  size = atoi(argv[2]);
  if ((buf = (char *)malloc(size)) == NULL) {
    fprintf(stderr, "%s\n", "Allocate error!");
  }
  // allocate 'A' into the whole space
  memset(buf, 'A', size);

  /* Connect to MongoDB server */
  connectToMongoDBServerWithURI(uri_string);

  start = clock();
  query = bson_new();
  cursor = mongoc_collection_find_with_opts(collection, query, NULL, NULL);
  while (mongoc_cursor_next(cursor, &doc)) {
    query_str = bson_as_json(doc, NULL);
    printf("%s\n", query_str);
    bson_free(query_str);
  }

  end = clock();
  duration = ((double) (end - start)) / CLOCKS_PER_SEC;

  printf("Throughput of create_link (MongoDB): %lf op/s\n",
          NUM_OPS / duration);

  /* Disconnect from MongoDB server */
  disconnectFromMongoDBServer();

  return EXIT_SUCCESS;
}
