#ifndef __IO_MONGO_H__
#define __IO_MONGO_H__

#include <mongoc/mongoc.h>
#include "common.h"

#define MAX_DOC_SIZE (8 * 1024 * 1024)

int connectToMongoDBServer();
int connectToMongoDBServerMPI(char *hostname);
int connectToMongoDBServerWithURI(char *uri_string);
void disconnectFromMongoDBServer();
void cleanMongoDB();
void write_mongo(char *key_str,
                 char *buf);
void write_mongo_sized(char *key_str,
                       char *buf,
                       int length);
void read_mongo_sized(char *key_str,
                      void *buf,
                      int length);
void create_symbios_metadata_mongo(symbios_metadata *metadata);
symbios_metadata *query_symbios_metadata_by_id_mongo(char *data_id);

#endif
