//
// Created by Jie on 8/25/20.
//

#include "mongo_io.h"
#include <string.h>

MongoIOClient::MongoIOClient()
{
    instance = std::make_shared<mongocxx::instance>();
    mongocxx::uri uri("mongodb://localhost:27017");
    mongocxx::client client(uri);
    coll = client["mydb"]["test"];
}

void MongoIOClient::Read(Data &source, Data &destination) {
    // create the filter document and then call the find_one method to query data from mongodb
    try {
        auto builder = bason::builder::stream::document{};
        bsoncxx::document::value query_doc_value = builder
                << "_id" << source.id_.c_str()
                << bsoncxx::builder::stream::finalize;
        bsoncxx::stdx::optional<bsoncxx::document::value> result = coll.find_one(query_doc_value);
        if(result){
            // get the result
            std::string value = bsoncxx::to_json(*maybe_result);
            // store the information in destination
            memcpy(destination.buffer_, value.c_str(), value.length());
            destination.data_size_ = value.length();
            // return successful
        }
        else
        {
            // return error
        }
    } catch (mongocxx::query_exception ex){
        // return error
    }
}

void MongoIOClient::Write(Data &source, Data &destination) {
    // create the json document and then call the collection insert method to write data into mongodb
    try {
        auto builder = bason::builder::stream::document{};
        bsoncxx::document::value insert_doc_value = builder
                << "_id" << source.id_.c_str()
                << "query_id" << source.id_.c_str()
                << source.id_.c_str() << (char*)source.buffer_
                << bsoncxx::builder::stream::finalize;
        coll.insert_one(insert_doc_value.view());
        // return successful
    } catch (mongocxx::bulk_write_exception ex){
        // return error
    }
}