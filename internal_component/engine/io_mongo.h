#ifndef __IO_MONGO_H__
#define __IO_MONGO_H__

#include <mongoc/mongoc.h>

#define MAX_DOC_SIZE (8 * 1024 * 1024)

int connectToMongoDBServer();
int connectToMongoDBServerMPI(char *hostname);
int connectToMongoDBServerWithURI(char *uri_string);
void disconnectFromMongoDBServer();
void cleanMongoDB();
void write_mongo(mongoc_collection_t *collection,
                 char *key_str,
                 char *buf);
void write_mongo_sized(mongoc_collection_t *collection,
                       char *key_str,
                       char *buf,
                       int length);
void read_mongo_sized(mongoc_collection_t *collection,
                      char *key_str,
                      void *buf,
                      int length);

#endif
