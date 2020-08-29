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
#include <memory>
#include "io.h"

class MongoIOClient: public IOClient {
public:
    /*
     * Constructor
     */
    MongoIOClient(uint16_t storage_index):IOClient(storage_index){
        auto mongo_solution = std::static_pointer_cast<MongoSS>(solution);
        instance = std::make_shared<mongocxx::instance>();
        mongocxx::uri uri(mongo_solution->end_point_.c_str());
        mongocxx::client client(uri);
        coll = client[mongo_solution->database_.c_str()][mongo_solution->collection_.c_str()];
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

private:
    std::shared_ptr<mongocxx::instance>   instance;
    mongocxx::collection coll;
};

#endif //SYMBIOS_MONGO_IO_H
