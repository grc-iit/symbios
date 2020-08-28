//
// Created by Jie on 8/25/20.
//

#include <symbios/io_clients/mongo_io.h>
#include <basket/common/singleton.h>
#include <symbios/common/configuration_manager.h>
#include <symbios/common/error_codes.h>
#include <cstring>

MongoIOClient::MongoIOClient() {
    ConfigurationManager conf = Singleton<ConfigurationManager>::GetInstance();

    instance = std::make_shared<mongocxx::instance>();
    mongocxx::uri uri(conf.mongodb_cluster_url);
    mongocxx::client client(uri);
    coll = client[conf.mongodb_cluster_database][conf.mongodb_cluster_collection];
}

void MongoIOClient::Read(Data &source, Data &destination) {
    // create the filter document and then call the find_one method to query data from mongodb

    // may have the partial write /read
    try {
        auto builder = bason::builder::stream::document{};
        bsoncxx::document::value query_doc_value = builder
                << "_id" << source.id_.c_str()
                << bsoncxx::builder::stream::finalize;
        bsoncxx::stdx::optional<bsoncxx::document::value> result = coll.find_one(query_doc_value);
        if(result){
            // get the result
            std::string value = bsoncxx::to_json(*maybe_result);
            std::string::size_type value_size = value.length();
            if(value_size == source.data_size_) {
                // store the information in destination
                memcpy(destination.buffer_, value.c_str(), value_size);
                destination.data_size_ = value_size;
            } else {
                throw ErrorException(READ_REDIS_POSITION_OR_SIZE_FAILED);
            }
        } else {
            throw ErrorException(READ_REDIS_POSITION_OR_SIZE_FAILED);
        }

    } catch (mongocxx::query_exception ex){
        throw ErrorException(READ_REDIS_POSITION_OR_SIZE_FAILED);
    }
}

void MongoIOClient::Write(Data &source, Data &destination) {
    // create the json document and then call the collection insert method to write data into mongodb
    try {
        std::string value = std::string((char*)source.buffer_, source.data_size_);
        auto builder = bason::builder::stream::document{};
        bsoncxx::document::value insert_doc_value = builder
                << "_id" << source.id_.c_str()
                << "query_id" << source.id_.c_str()
                << source.id_.c_str() << value
                << bsoncxx::builder::stream::finalize;
        coll.insert_one(insert_doc_value.view());
    } catch (mongocxx::bulk_write_exception ex){
        throw ErrorException(READ_REDIS_POSITION_OR_SIZE_FAILED);
    }
}
