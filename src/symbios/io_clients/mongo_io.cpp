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
    mongocxx::collection file = client[mongo_solution->database_.c_str()].collection(
            mongo_solution->collection_.c_str());
    bsoncxx::stdx::optional<bsoncxx::document::value> maybe_result =
            file.find_one(bsoncxx::builder::stream::document{} << "key" << std::string(source.id_.c_str())
                                                               << bsoncxx::builder::stream::finalize);
    if (maybe_result) {
        destination.buffer_= maybe_result->view()["value"].get_utf8().value.to_string();
    } else {
        throw ErrorException(READ_REDIS_DATA_FAILED);
    }
}

void MongoIOClient::Write(Data &source, Data &destination) {
    mongocxx::collection file = client[mongo_solution->database_.c_str()].collection(
            mongo_solution->collection_.c_str());
    bool exists = false;
    Data read_source;
    try {
        read_source.id_ = destination.id_;
        Read(read_source, read_source);
        exists = true;
    } catch (const std::exception &e) {
        exists = false;
    }
    bool delete_source_buffer = false;
    if (exists) {
        if (source.buffer_.size() - source.position_ >= read_source.buffer_.size() || source.buffer_.size() - source.position_ + destination.position_ >= read_source.buffer_.size()) {
            auto new_val=std::string();
            new_val.resize(destination.position_ + source.buffer_.size() - source.position_);
            if (destination.position_ > 0) {
                memcpy(new_val.data(), read_source.buffer_.c_str(), destination.position_ - 1);
            }
            memcpy(new_val.data() + destination.position_, source.buffer_.c_str() + source.position_, source.buffer_.size() - source.position_);
            source.buffer_ = new_val;
            source.position_ = 0;
            delete_source_buffer = true;
        } else {
            // update the old_value
            memcpy(read_source.buffer_.data() + destination.position_,
                   source.buffer_.c_str() + source.position_,
                   source.buffer_.size() - source.position_);
            source.position_ = 0;
            source.buffer_ = read_source.buffer_;
        }
        file.delete_many(bsoncxx::builder::basic::make_document(
                bsoncxx::builder::basic::kvp("key", std::string(destination.id_.c_str()))));
    }
    auto document = bsoncxx::builder::basic::document{};
    using bsoncxx::builder::basic::kvp;
    std::string data(source.buffer_.c_str() + source.position_, source.buffer_.size() - source.position_);
    std::string keyName(destination.id_.c_str());
    //std::cout<<"KeyName :"<<keyName<<"\n";
    document.append(kvp("key", keyName), kvp("value", data));

    //adding the created key-value pair to the collection
    bsoncxx::document::view putView = document.view();//get the view
    //retrieve the unique objectID from map
    auto add = file.insert_one(putView);//insert it to collection
    if (!add) {
        std::cout << "Unacknowledged write. No id available." << "\n";
    }
    if (add->inserted_id().type() == bsoncxx::type::k_oid) {
        bsoncxx::oid id = add->inserted_id().get_oid().value;
    } else std::cout << "Inserted id was not an OID type" << "\n";
}

void MongoIOClient::Remove(Data &source) {
    mongocxx::collection file = client[mongo_solution->database_.c_str()].collection(
            mongo_solution->collection_.c_str());
    file.delete_many(bsoncxx::builder::basic::make_document(
            bsoncxx::builder::basic::kvp("key", std::string(source.id_.c_str()))));
}
