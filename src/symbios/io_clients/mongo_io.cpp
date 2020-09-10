//
// Created by Jie on 8/25/20.
//

#include "common/debug.h"
#include <basket/common/singleton.h>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <common/debug.h>
#include <cstring>
#include <mongocxx/exception/bulk_write_exception.hpp>
#include <mongocxx/exception/query_exception.hpp>
#include <string>
#include <symbios/common/configuration_manager.h>
#include <symbios/common/error_codes.h>
#include <symbios/io_clients/mongo_io.h>

/*
 * Reads data from source into destination buffer while respecting the position_
 * @parameter source: describe the key-value related information which you want to read from mongo
 * @parameter destination: the memory information
 */
void MongoIOClient::Read(Data &source, Data &destination) {
  AUTO_TRACER("MongoIOClient::Read", source, destination);
mongocxx::collection file = client[mongo_solution->database_.c_str()].collection(
            mongo_solution->collection_.c_str());
    bsoncxx::stdx::optional<bsoncxx::document::value> maybe_result =
            file.find_one(bsoncxx::builder::stream::document{} << "key" << std::string(source.id_.c_str())
                                                               << bsoncxx::builder::stream::finalize);
    if (maybe_result) {
        auto data = maybe_result->view()["value"].get_utf8().value.to_string();
        size_t source_size = source.data_size_;
        if(source_size == 0){
            source_size = data.size();
        }
        destination.buffer_= static_cast<char *>(malloc(source_size) );
        memcpy(destination.buffer_,data.data()+source.position_,source_size - source.position_);
        destination.data_size_ = source_size;
    } else {
        throw ErrorException(READ_MONGODB_DATA_FAILED);
    }
    COMMON_DBGVAR(destination);
}

/*
 * Writes data from source into destination buffer while respecting the position_
 * 1) If the key is non exist in mongo, just write the key-value information into mongo
 * 2) If the key has been exist in mongo, update the data and then put back to mongo
 * @parameter source: the memory information which stores the data you want to write to mongo
 * @parameter destination: the redis information which has the key information
 */
void MongoIOClient::Write(Data &source, Data &destination) {
    AUTO_TRACER("MongoIOClient::Write", source,destination);
    mongocxx::collection file = client[mongo_solution->database_.c_str()].collection(
            mongo_solution->collection_.c_str());
    bool exists = false;
    Data read_source;
    std::string new_val;
    try {
        read_source.id_ = destination.id_;
        Read(read_source, read_source);
        exists = true;
    } catch (const std::exception &e) {
        exists = false;
    }
    if (exists) {
        if (source.data_size_  >= read_source.data_size_ || source.data_size_  + destination.position_ >= read_source.data_size_) {
            new_val=std::string();
            new_val.resize(destination.position_ + source.data_size_ );
            if (destination.position_ > 0) {
                memcpy(new_val.data(), read_source.buffer_, destination.position_ > read_source.data_size_? read_source.data_size_:destination.position_ - 1);
            }
            memcpy(new_val.data() + destination.position_, source.buffer_ + source.position_, source.data_size_);
            source.position_ = 0;
        } else {
            new_val=std::string(read_source.buffer_,read_source.data_size_);
            // update the old_value
            memcpy(new_val.data() + destination.position_,
                   source.buffer_ + source.position_,
                   source.data_size_ );
            source.position_ = 0;
        }
        file.delete_many(bsoncxx::builder::basic::make_document(
                bsoncxx::builder::basic::kvp("key", std::string(destination.id_.c_str()))));
        free(read_source.buffer_);
    }else{
        new_val=std::string(source.buffer_+ source.position_,source.data_size_);
    }
    auto document = bsoncxx::builder::basic::document{};
    using bsoncxx::builder::basic::kvp;
    std::string keyName(destination.id_.c_str());
    //std::cout<<"KeyName :"<<keyName<<"\n";
    document.append(kvp("key", keyName), kvp("value", new_val));

    //adding the created key-value pair to the collection
    bsoncxx::document::view putView = document.view();//get the view
    //retrieve the unique objectID from map
    auto add = file.insert_one(putView);//insert it to collection
    if (!add) {
        std::cout << "Unacknowledged write. No id available." << "\n";
    }
    if (add->inserted_id().type() == bsoncxx::type::k_oid) {
        bsoncxx::oid id = add->inserted_id().get_oid().value;
        COMMON_DBGVAR(id.to_string());
    } else std::cout << "Inserted id was not an OID type" << "\n";
}

/*
 * Remove the data from mongo io storage
 * @parameter source: the data request information which contains the key you want to remove from mongo
 * @return bool
 */
bool MongoIOClient::Remove(Data &source) {
    AUTO_TRACER("MongoIOClient::Remove", source);
    mongocxx::collection file = client[mongo_solution->database_.c_str()].collection(
            mongo_solution->collection_.c_str());
    file.delete_many(bsoncxx::builder::basic::make_document(
            bsoncxx::builder::basic::kvp("key", std::string(source.id_.c_str()))));
    return true;
}

/*
 * Get the data size
 * @parameter source: the data request information which contains the key information
 * @return size_t: if the key is exist in mongo, return the data size; if the key is non exist, return 0
 */
size_t MongoIOClient::Size(Data &source) {
    bool exists = false;
    Data read_source;
    try {
        read_source.id_ = source.id_;
        Read(read_source, read_source);
        return read_source.data_size_;
    } catch (const std::exception &e) {
       return 0;
    }
}
