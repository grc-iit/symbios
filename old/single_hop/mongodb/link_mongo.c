#include <mongoc/mongoc.h>
#include "../xattr/common.h"
#include "link_mongo.h"

int connectToMongoDBServer()
{
  char uri_string[1024]; //= "mongodb://172.25.101.1:27017";

  memset(uri_string, 0, 1024);
  sprintf(uri_string, "mongodb://%s:%d", MONGODB_HOST, MONGODB_PORT);
  return connectToMongoDBServerWithURI(uri_string);
}

int connectToMongoDBServerMPI(char *hostname)
{
  char uri_string[1024]; //= "mongodb://172.25.101.1:27017";

  memset(uri_string, 0, 1024);
  sprintf(uri_string, "mongodb://%s:%d", hostname, MONGODB_PORT);
  return connectToMongoDBServerWithURI(uri_string);
}

int connectToMongoDBServerWithURI(char *uri_string)
{
  bson_t *command, reply, *insert;
  bson_error_t error;
  mongoc_client_t *client;
  mongoc_uri_t *uri;
  char *str;
  bool retval;

  /*
   * Required to initialize libmongoc's internals
   */
  mongoc_init();

  /*
   * Safely create a MongoDB URI object from the given string
   */
  uri = mongoc_uri_new_with_error(uri_string, &error);
  if (!uri) {
    fprintf(stderr,
        "failed to parse URI: %s\n"
        "error message:       %s\n",
        uri_string,
        error.message);
    return EXIT_FAILURE;
  }

  /*
   * Create a new client instance
   */
  client = mongoc_client_new_from_uri(uri);
  if (!client) {
    return EXIT_FAILURE;
  }

  /*
   * Register the application name so we can track it in the profile logs
   * on the server. This can also be done from the URI (see other examples).
   */
  mongoc_client_set_appname(client, "connect-example");

  /*
   * Do work. This example pings the database, prints the result as JSON and
   * performs an insert
   */
  command = BCON_NEW("ping", BCON_INT32(1));

  retval = mongoc_client_command_simple(
      client, "admin", command, NULL, &reply, &error);

  if (!retval) {
    fprintf(stderr, "%s\n", error.message);
    return EXIT_FAILURE;
  }

  str = bson_as_json(&reply, NULL);
  /*printf("reply: %s\n", str);*/
  bson_destroy(command);
  bson_destroy(insert);
  bson_destroy(&reply);
  bson_free(str);
  mongoc_uri_destroy(uri);

  /*
   * Get a handle on the database "db_name" and collection "coll_name"
   */
  database = mongoc_client_get_database(client, MONGODB_DB_NAME);
  collection = mongoc_client_get_collection(client,
                                            MONGODB_DB_NAME,
                                            MONGODB_COLL_NAME);
}

void disconnectFromMongoDBServer()
{
  /*
   * Release our handles and clean up libmongoc
   */
  mongoc_cleanup();
}

void cleanMongoDB()
{
  bson_error_t error;

  if (!mongoc_database_drop(database,&error)) {
    fprintf(stderr, "%s\n", error.message);
  }
}

void create_link_mongo(mongoc_collection_t *collection, char *key_str)
{
  bson_t *insert;
  bson_error_t error;

  insert = BCON_NEW(key_str, "mongodb");
  if (!mongoc_collection_insert_one(collection, insert, NULL, NULL, &error)) {
    fprintf(stderr, "%s\n", error.message);
  }
}

void create_return_link_mongo(mongoc_collection_t *collection,
                              char *key_str,
                              char *target)
{
  bson_t *insert;
  bson_error_t error;

  insert = BCON_NEW(key_str, target);
  if (!mongoc_collection_insert_one(collection, insert, NULL, NULL, &error)) {
    fprintf(stderr, "%s\n", error.message);
  }
}

void create_link_mongo_sized(mongoc_collection_t *collection,
                             char *key_str,
                             char *buf)
{
  bson_t *insert;
  bson_error_t error;

  insert = BCON_NEW(key_str, buf);
  if (!mongoc_collection_insert_one(collection, insert, NULL, NULL, &error)) {
    fprintf(stderr, "%s\n", error.message);
  }
}

