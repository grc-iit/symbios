//
// Created by Jie on 8/25/20.
//

#include <symbios/io_clients/mongo_io.h>
#include <basket/common/singleton.h>
#include <symbios/common/configuration_manager.h>
#include <symbios/common/error_codes.h>
#include <cstring>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <mongocxx/exception/query_exception.hpp>
#include <mongocxx/exception/bulk_write_exception.hpp>


void MongoIOClient::Read(Data &source, Data &destination) {
    try {
        auto builder = bason::builder::stream::document{};
        bsoncxx::view_or_value<bsoncxx::document::view, bsoncxx::document::value> query_doc_value = builder
                << "_id" << source.id_.c_str()
                << bsoncxx::builder::stream::finalize;
        bsoncxx::stdx::optional<bsoncxx::document::value> result = coll.find_one(query_doc_value);
        if(result){
            // get the result
            std::string value = bsoncxx::to_json(*maybe_result);
            std::string::size_type value_size = value.length();
            if(source.position_ + source.data_size_ > value_size){
                throw ErrorException(READ_MONGO_POSITION_OR_SIZE_FAILED);
            }
            else {
                // read data from mogodb to the memory buffer
                memcpy(destination.buffer_, value.c_str() + source.position_, source.data_size_);
                destination.data_size_ = source.data_size_;
            }
        } else {
            throw ErrorException(READ_MONGODB_DATA_FAILED);
        }

    } catch (mongocxx::query_exception ex){
        throw ErrorException(MONGODB_SERVER_SIDE_FAILED);
    }
}

void MongoIOClient::Write(Data &source, Data &destination) {
    try {
        auto builder = bason::builder::stream::document{};
        bsoncxx::view_or_value<bsoncxx::document::view, bsoncxx::document::value> query_doc_value = builder
                << "_id" << source.id_.c_str()
                << bsoncxx::builder::stream::finalize;
        bsoncxx::stdx::optional<bsoncxx::document::value> result = coll.find_one(query_doc_value);
        if(result){
            // The key has been existed in Mongodb
            std::string old_value = bsoncxx::to_json(*maybe_result);
            std::string::size_type old_value_size = value.length();
            if (source.data_size_ >= old_value_size){
                std::string new_value = std::string((char*)source.buffer_ + source.position_, source.data_size_);
                auto builder = bason::builder::stream::document{};
                bsoncxx::document::value insert_doc_value = builder
                        << "_id" << source.id_.c_str()
                        << source.id_.c_str() << new_value
                        << bsoncxx::builder::stream::finalize;
                coll.insert_one(insert_doc_value.view());
                destination.data_size_ = source.data_size_;
            }
            else {
                // update the old_value
                memcpy((void*)old_value.c_str(), (const void*)((char*)source.buffer_ + source.position_), source.data_size_);
                // put the updated data back
                auto builder = bason::builder::stream::document{};
                bsoncxx::document::value insert_doc_value = builder
                        << "_id" << source.id_.c_str()
                        << source.id_.c_str() << new_value
                        << bsoncxx::builder::stream::finalize;
                coll.insert_one(insert_doc_value.view());
                destination.data_size_ = source.data_size_;
            }
        }
        else {
            // The key isn't exist in redis cluster
            std::string value = std::string((char*)source.buffer_ + source.position_, source.data_size_);
            auto builder = bason::builder::stream::document{};
            bsoncxx::document::value insert_doc_value = builder
                    << "_id" << source.id_.c_str()
                    << source.id_.c_str() << value
                    << bsoncxx::builder::stream::finalize;
            coll.insert_one(insert_doc_value.view());
            destination.data_size_ = source.data_size_;
        }

    } catch (mongocxx::bulk_write_exception ex){
        throw ErrorException(MONGODB_SERVER_SIDE_FAILED);
    }
}
