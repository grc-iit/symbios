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
#include <symbios/common/configuration_manager.h>
#include <symbios/common/error_codes.h>
#include <symbios/io_clients/mongo_io.h>

void MongoIOClient::Read(Data &source, Data &destination) {
  auto tracer_source =
      common::debug::AutoTrace(std::string("MongoIOClient::Read"), source);
  auto tracer_destination =
      common::debug::AutoTrace(std::string("MongoIOClient::Read"), destination);
  mongocxx::collection file =
      client[mongo_solution->database_.c_str()].collection(
          mongo_solution->collection_.c_str());
  bsoncxx::stdx::optional<bsoncxx::document::value> maybe_result =
      file.find_one(bsoncxx::builder::stream::document{}
                    << "key" << std::string(source.id_.c_str())
                    << bsoncxx::builder::stream::finalize);
  if (maybe_result) {
    std::string_view value = maybe_result.value()
                                 .view()["value"]
                                 .get_value()
                                 .get_utf8()
                                 .value.to_string();
    destination.buffer_ = malloc(value.size() + 1);
    memcpy(destination.buffer_, value.data(), value.size() + 1);
    destination.data_size_ = value.size() + 1;
  } else {
    throw ErrorException(READ_REDIS_DATA_FAILED);
  }
}

void MongoIOClient::Write(Data &source, Data &destination) {
  auto tracer_source =
      common::debug::AutoTrace(std::string("MongoIOClient::Write"), source);
  auto tracer_destination = common::debug::AutoTrace(
      std::string("MongoIOClient::Write"), destination);

  mongocxx::collection file =
      client[mongo_solution->database_.c_str()].collection(
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
  if (exists) {
    char *old_value = (char *)read_source.buffer_;
    size_t old_value_size = read_source.data_size_;
    if (source.data_size_ >= old_value_size ||
        source.data_size_ + destination.position_ >= old_value_size) {
      char *new_val = (char *)malloc(destination.position_ + source.data_size_);
      if (destination.position_ > 0) {
        memcpy(new_val, old_value, destination.position_ - 1);
      }
      memcpy(new_val + destination.position_, source.buffer_ + source.position_,
             source.data_size_);
      delete (old_value);
      delete (source.buffer_);
      source.data_size_ = destination.position_ + source.data_size_;
      source.buffer_ = new_val;
      source.position_ = 0;
      destination.data_size_ = source.data_size_;
    } else {
      // update the old_value
      memcpy(old_value + destination.position_,
             (const void *)((char *)source.buffer_ + source.position_),
             source.data_size_);
      delete (source.buffer_);
      source.data_size_ = source.data_size_;
      source.position_ = 0;
      source.buffer_ = old_value;
      destination.data_size_ = source.data_size_;
    }
    file.delete_many(
        bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp(
            "key", std::string(destination.id_.c_str()))));
  }
  auto document = bsoncxx::builder::basic::document{};
  using bsoncxx::builder::basic::kvp;
  std::string data((const char *)source.buffer_ + source.position_,
                   source.data_size_);
  std::string keyName(destination.id_.c_str());
  // std::cout<<"KeyName :"<<keyName<<"\n";
  document.append(kvp("key", keyName), kvp("value", data));

  // adding the created key-value pair to the collection
  bsoncxx::document::view putView = document.view(); // get the view
  // retrieve the unique objectID from map
  auto add = file.insert_one(putView); // insert it to collection
  if (!add) {
    std::cout << "Unacknowledged write. No id available."
              << "\n";
  }
  if (add->inserted_id().type() == bsoncxx::type::k_oid) {
    bsoncxx::oid id = add->inserted_id().get_oid().value;
  } else
    std::cout << "Inserted id was not an OID type"
              << "\n";
}

void MongoIOClient::Remove(Data &source) {
  mongocxx::collection file =
      client[mongo_solution->database_.c_str()].collection(
          mongo_solution->collection_.c_str());
  file.delete_many(bsoncxx::builder::basic::make_document(
      bsoncxx::builder::basic::kvp("key", std::string(source.id_.c_str()))));
}
