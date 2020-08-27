//
// Created by Jie on 8/25/20.
// It is the implementation of Mongo IO
//

#ifndef SYMBIOS_MONGO_IO_H
#define SYMBIOS_MONGO_IO_H

#include "key_value_store_io.h"
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/instance.hpp>

class MongoIOClient: public KeyValueStoreIO {
public:
    /*
     * Constructor
     */
    MongoIOClient();
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

private:
    std::shared_ptr<mongocxx::instance>   instance;
    mongocxx::collection coll;
};

#endif //SYMBIOS_MONGO_IO_H
