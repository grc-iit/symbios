#ifndef __LINK_MONGO_H__
#define __LINK_MONGO_H__

#include <mongoc/mongoc.h>

mongoc_database_t *database;
mongoc_collection_t *collection;

int connectToMongoDBServer();
int connectToMongoDBServerMPI(char *hostname);
int connectToMongoDBServerWithURI(char *uri_string);
void disconnectFromMongoDBServer();
void cleanMongoDB();
void create_link_mongo(mongoc_collection_t *collection,
                       char *key_str);
void create_return_link_mongo(mongoc_collection_t *collection,
                              char *key_str,
                              char *target);
void create_link_mongo_sized(mongoc_collection_t *collection,
                             char *key_str,
                             char *buf);

#endif
