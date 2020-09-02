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
            common::debug::AutoTrace(std::string("RedisIOClient::Read"), source, destination);
    try {

        auto resp = m_redisCluster->get(source.id_.c_str());
        if (resp) {
            std::string value = *resp;
            std::string::size_type value_size = value.length();
            if (source.buffer_.size() > value_size) {
                throw ErrorException(READ_REDIS_POSITION_OR_SIZE_FAILED);
            } else {
                // read data from Redis successful
                destination.buffer_ = std::string(value.c_str() + source.position_, source.buffer_.size());
            }
        } else {
            throw ErrorException(READ_REDIS_DATA_FAILED);
        }
    } catch (const Error &err) {
        throw ErrorException(REDIS_SERVER_SIDE_FAILED);
    }
    COMMON_DBGVAR(destination);
}

void RedisIOClient::Write(Data &source, Data &destination) {
    auto tracer_source =
            common::debug::AutoTrace("RedisIOClient::Write", source, destination);
    try {
        auto resp = m_redisCluster->get(destination.id_.c_str());
        if (resp) {
            // The key has been existed in redis cluster.
            std::string old_value = *resp;
            std::string::size_type old_value_size = old_value.length();
            if (source.buffer_.size() - source.position_ >= old_value_size ||
                source.buffer_.size() - source.position_ + destination.position_ >= old_value_size) {
                auto new_val = std::string();
                new_val.resize(destination.position_ + source.buffer_.size() - source.position_);
                if (destination.position_ > 0) {
                    memcpy(new_val.data(), old_value.c_str(), destination.position_ - 1);
                }
                memcpy(new_val.data() + destination.position_, source.buffer_.data() + source.position_, source.buffer_.size() -  source.position_);
                bool result = m_redisCluster->set(destination.id_.c_str(), new_val);
                if (!result) {
                    throw ErrorException(WRITE_REDIS_DATA_FAILED);
                }
                destination.buffer_ = new_val;
            } else {
                // update the old_value
                memcpy(old_value.data() + destination.position_,
                       source.buffer_.c_str() + source.position_,
                       source.buffer_.size() - source.position_);
                // put the updated data back
                bool result = m_redisCluster->set(destination.id_.c_str(), old_value);
                if (!result) {
                    throw ErrorException(WRITE_REDIS_DATA_FAILED);
                }
                destination.buffer_ = old_value;
            }
        } else {
            // The key isn't exist in redis cluster
            std::string value = std::string(source.buffer_.c_str() + source.position_, source.buffer_.size() - source.position_);
            bool result = m_redisCluster->set(destination.id_.c_str(), value);
            if (!result) {
                throw ErrorException(WRITE_REDIS_DATA_FAILED);
            }
            destination.buffer_ = value;
        }

    } catch (const Error &err) {
        throw ErrorException(REDIS_SERVER_SIDE_FAILED);
    }
}

void RedisIOClient::Remove(Data &source) {
    auto tracer_source = common::debug::AutoTrace("RedisIOClient::Remove", source);
    auto resp = m_redisCluster->del(source.id_.c_str());

}

size_t RedisIOClient::Size(Data &source) {
    auto resp = m_redisCluster->get(source.id_.c_str());
    if(resp){
        return resp->size();
    }else return 0;
}
