//
// Created by Jie on 8/25/20.
//

#include <basket/common/singleton.h>
#include <common/debug.h>
#include <cstring>
#include <string>
#include <symbios/common/configuration_manager.h>
#include <symbios/common/error_codes.h>
#include <symbios/io_clients/redis_io.h>

void RedisIOClient::Read(Data &source, Data &destination) {
  auto tracer_source =
      common::debug::AutoTrace(std::string("RedisIOClient::Read"), source);
  auto tracer_destination =
      common::debug::AutoTrace(std::string("RedisIOClient::Read"), destination);
  try {
    auto resp = m_redisCluster->get(source.id_.c_str());
    if (resp) {
      std::string value = *resp;
      std::string::size_type value_size = value.length();
      if (source.position_ + source.data_size_ > value_size) {
        throw ErrorException(READ_REDIS_POSITION_OR_SIZE_FAILED);
      } else {
        // read data from Redis successful
        memcpy(destination.buffer_, value.c_str() + source.position_,
               source.data_size_);
        destination.data_size_ = source.data_size_;
      }
    } else {
      throw ErrorException(READ_REDIS_DATA_FAILED);
    }
  } catch (const Error &err) {
    throw ErrorException(REDIS_SERVER_SIDE_FAILED);
  }
}

void RedisIOClient::Write(Data &source, Data &destination) {
  auto tracer_source =
      common::debug::AutoTrace(std::string("RedisIOClient::Write"), source);
  auto tracer_destination = common::debug::AutoTrace(
      std::string("RedisIOClient::Write"), destination);
  try {
    auto resp = m_redisCluster->get(destination.id_.c_str());
    if (resp) {
      // The key has been existed in redis cluster.
      std::string old_value = *resp;
      std::string::size_type old_value_size = old_value.length();
      if (source.data_size_ >= old_value_size ||
          source.data_size_ + destination.position_ >= old_value_size) {
        char *new_val =
            (char *)malloc(destination.position_ + source.data_size_);
        if (destination.position_ > 0) {
          memcpy(new_val, old_value.c_str(), destination.position_ - 1);
        }
        memcpy(new_val + destination.position_,
               source.buffer_ + source.position_, source.data_size_);
        std::string new_value = std::string(
            (char *)new_val, destination.position_ + source.data_size_);
        bool result = m_redisCluster->set(destination.id_.c_str(), new_value);
        if (!result) {
          throw ErrorException(WRITE_REDIS_DATA_FAILED);
        }
        destination.data_size_ = destination.position_ + source.data_size_;
      } else {
        // update the old_value
        memcpy((void *)old_value.c_str() + destination.position_,
               (const void *)((char *)source.buffer_ + source.position_),
               source.data_size_);
        // put the updated data back
        bool result = m_redisCluster->set(destination.id_.c_str(), old_value);
        if (!result) {
          throw ErrorException(WRITE_REDIS_DATA_FAILED);
        }
        destination.data_size_ = source.data_size_;
      }
    } else {
      // The key isn't exist in redis cluster
      std::string value = std::string(
          (char *)(source.buffer_ + source.position_), source.data_size_);
      bool result = m_redisCluster->set(destination.id_.c_str(), value);
      if (!result) {
        throw ErrorException(WRITE_REDIS_DATA_FAILED);
      }
      destination.data_size_ = source.data_size_;
    }

  } catch (const Error &err) {
    throw ErrorException(REDIS_SERVER_SIDE_FAILED);
  }
}

void RedisIOClient::Remove(Data &source) {}
