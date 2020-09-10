//
// Created by Jie on 8/25/20.
// It is the implementation of Mongo IO
//

#ifndef SYMBIOS_MONGO_IO_H
#define SYMBIOS_MONGO_IO_H

#include <symbios/io_clients/io.h>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/instance.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <mongocxx/logger.hpp>
#include <bsoncxx/stdx/make_unique.hpp>
#include <mongocxx/exception/exception.hpp>
#include <memory>
#include "io.h"

/*
 * An subclass which inherits from IOClient class
 * 1) implement the interface to access mongo io storage
 */
class MongoIOClient: public IOClient {
public:
    /*
     * Constructor
     */
    MongoIOClient(uint16_t storage_index):IOClient(storage_index){
        mongo_solution = std::static_pointer_cast<MongoSS>(solution);
        class noop_logger : public mongocxx::logger {
        public:
            virtual void operator()(mongocxx::log_level,
                                    mongocxx::stdx::string_view,
                                    mongocxx::stdx::string_view) noexcept {}
        };
        /* Setup the MongoDB client */
        mongocxx::instance instance{mongocxx::stdx::make_unique<noop_logger>()};
        auto uri = mongocxx::uri{mongo_solution->end_point_.c_str()};
        try {
            client = mongocxx::client{uri};
            /* make sure the client is OK */
            if (!client) {
                fprintf(stderr, "Cannot create MongoDB client.\n");
                throw ErrorException(CREATE_MONGO_CLIENT_ERROR);
            }
            mongocxx::database db = client[mongo_solution->database_.c_str()];
            if (!db) {
                fprintf(stderr, "Cannot connect to MongoDB database.\n");
                throw ErrorException(CONNECT_MONGO_DATABASE_ERROR);
            }
            mongocxx::collection file;
            if(BASKET_CONF->MPI_RANK==0) {
                file = client.database(mongo_solution->database_.c_str()).has_collection(mongo_solution->collection_.c_str()) ?
                   db.collection(mongo_solution->collection_.c_str()) :
                   db.create_collection(mongo_solution->collection_.c_str());
            }
            file = db.collection(mongo_solution->collection_.c_str());
            if (!file) {
                fprintf(stderr, "Cannot connect to MongoDB collection.\n");
                throw ErrorException(CONNECT_MONGO_COLLECTION_ERROR);
            }
        } catch (const mongocxx::exception& e){
            throw ErrorException(CONNECT_MONGO_SERVER_ERROR);
        }
    }
    /*
     * Methods
     */

    /*
     * Reads data from source into destination buffer while respecting the position_
     */
    void Read(Data &source, Data &destination) override;

    /*
     * Writes data from source into destination buffer while respecting the position_
     */
    void Write(Data &source, Data &destination) override;

    /*
     * Remove data interface
     */
    bool Remove(Data &source) override;

    /*
     * Get data size interface
     */
    size_t Size(Data &source) override;

private:
    mongocxx::client client;
    std::shared_ptr<MongoSS> mongo_solution;
};

#endif //SYMBIOS_MONGO_IO_H
