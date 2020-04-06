#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <mongoc/mongoc.h>
#include "common.h"
#include "io_mongo.h"

mongoc_database_t *database = NULL;
mongoc_collection_t *collection = NULL;

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
  if (collection != NULL);
    mongoc_cleanup();
}

void cleanMongoDB()
{
  bson_error_t error;

  if (!mongoc_database_drop(database,&error)) {
    fprintf(stderr, "%s\n", error.message);
  }
}

void write_mongo(mongoc_collection_t *collection, char *key_str, char *buf)
{
  bson_t *insert;
  bson_error_t error;
  insert = bson_new ();

  char id[30];
  pid_t process_id;
  process_id = getpid();
  // srand(process_id);
  sprintf(id, "%s%d%d", key_str, process_id, rand());
  BSON_APPEND_UTF8 (insert, "_id", id);
  BSON_APPEND_UTF8 (insert, key_str, "mongodb");
  // insert = BCON_NEW(key_str, "mongodb");
  if (!mongoc_collection_insert_one(collection, insert, NULL, NULL, &error)) {
    fprintf(stderr, "%s\n", error.message);
  }
}

void write_mongo_sized(mongoc_collection_t *collection,
                       char *key_str,
                       char *buf,
                       int length)
{
  bson_t *insert;
  bson_error_t error;
  char id[30], sub_key_str[1024];
  pid_t process_id;
  process_id = getpid();
  int sub_length, finished_length, count;
  // sprintf(id, "%s%d%d", key_str, process_id, rand());
  // sprintf(id, "%s", key_str);

  if (length > MAX_DOC_SIZE) {
    sub_length = MAX_DOC_SIZE;
    for (count = 0, finished_length = 0; finished_length < length; count++) {
      memset(sub_key_str, 0, 1024);
      sprintf(sub_key_str, "%s_%d", key_str, count);
      insert = bson_new ();
      BSON_APPEND_UTF8 (insert, "_id", sub_key_str);
      BSON_APPEND_UTF8 (insert,"queryid", sub_key_str);
      bson_append_utf8 (insert,
                        sub_key_str,
                        (int) strlen(sub_key_str),
                        buf + finished_length,
                        sub_length);
      if (!mongoc_collection_insert_one(collection,
                                        insert,
                                        NULL,
                                        NULL,
                                        &error)) {
        fprintf(stderr, "%s\n", error.message);
      }
      finished_length += sub_length;
      debug_print(2, "sub request inserted, length %d\n", sub_length);
      if (finished_length + MAX_DOC_SIZE > length)
        sub_length = length % MAX_DOC_SIZE;
    }
  } else {
    insert = bson_new ();
    BSON_APPEND_UTF8 (insert, "_id", key_str);
    BSON_APPEND_UTF8 (insert,"queryid", key_str);
    bson_append_utf8 (insert,
                      key_str,
                      (int) strlen(key_str),
                      buf,
                      length);
    if (!mongoc_collection_insert_one(collection,
                                      insert,
                                      NULL,
                                      NULL,
                                      &error)) {
      fprintf(stderr, "%s\n", error.message);
    }
  }
}

void read_mongo_sized(mongoc_collection_t *collection,
                      char *key_str,
                      void *buf,
                      int length)
{
  mongoc_cursor_t *cursor;
  const bson_t *doc;
  bson_t *query;
  int count, i;
  char sub_key_str[1024];

  if (length > MAX_DOC_SIZE) {
    count = ceil((double) length / MAX_DOC_SIZE);
    for (i = 0; i < count; i++) {
      memset(sub_key_str, 0, 1024);
      sprintf(sub_key_str, "%s_%d", key_str, i);
      query = bson_new();
      BSON_APPEND_UTF8(query, "_id", sub_key_str);
      cursor = mongoc_collection_find_with_opts (collection, query, NULL, NULL);
      while (mongoc_cursor_next (cursor, &doc)) {
        buf = bson_as_json (doc, NULL);
        debug_print(3, "sub res: %s\n", buf);
      }
      buf += MAX_DOC_SIZE;
    }
  } else {
    query = bson_new();
    BSON_APPEND_UTF8(query, "_id", key_str);
    cursor = mongoc_collection_find_with_opts (collection, query, NULL, NULL);
    // mongoc_cursor_next (cursor, &doc);

    while (mongoc_cursor_next (cursor, &doc)) {
      buf = bson_as_json (doc, NULL);
      debug_print(3, "sub res: %s\n", buf);
      debug_print(2, "ok\n");
    }
  }
  mongoc_cursor_destroy (cursor);
  bson_destroy (query);
}
