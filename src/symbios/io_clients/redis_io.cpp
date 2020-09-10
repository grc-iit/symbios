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
    AUTO_TRACER(std::string("RedisIOClient::Read"), source, destination);
    try {

        auto resp = m_redisCluster->get(source.id_.c_str());
        if (resp) {
            std::string value = *resp;
            std::string::size_type value_size = value.length();
            size_t source_size = source.data_size_;
            if(source_size == 0){
                source_size = value_size;
            }
            if (source_size > value_size) {
                throw ErrorException(READ_REDIS_POSITION_OR_SIZE_FAILED);
            } else {
                // read data from Redis successful
                destination.buffer_= static_cast<char *>(malloc(source_size));
                memcpy(destination.buffer_, value.c_str()+ source.position_,source_size );
                destination.data_size_=source_size;
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
    AUTO_TRACER("RedisIOClient::Write", source, destination);
    try {
        auto resp = m_redisCluster->get(destination.id_.c_str());
        if (resp) {
            // The key has been existed in redis cluster.
            std::string old_value = *resp;
            std::string::size_type old_value_size = old_value.length();
            if (source.data_size_ >= old_value_size ||
                source.data_size_ + destination.position_ >= old_value_size) {
                auto new_val = std::string();
                new_val.resize(destination.position_ + source.data_size_);
                if (destination.position_ > 0) {
                    memcpy(new_val.data(), old_value.c_str(), destination.position_ > old_value_size?old_value_size:destination.position_ - 1);
                }
                memcpy(new_val.data() + destination.position_, source.buffer_ + source.position_, source.data_size_);
                bool result = m_redisCluster->set(destination.id_.c_str(), new_val);
                if (!result) {
                    throw ErrorException(WRITE_REDIS_DATA_FAILED);
                }
            } else {
                // update the old_value
                memcpy(old_value.data() + destination.position_,
                       source.buffer_ + source.position_,
                       source.data_size_);
                // put the updated data back
                bool result = m_redisCluster->set(destination.id_.c_str(), old_value);
                if (!result) {
                    throw ErrorException(WRITE_REDIS_DATA_FAILED);
                }
            }
        } else {
            // The key isn't exist in redis cluster
            std::string value = std::string(source.buffer_ + source.position_,source.data_size_);
            bool result = m_redisCluster->set(destination.id_.c_str(), value);
            if (!result) {
                throw ErrorException(WRITE_REDIS_DATA_FAILED);
            }
        }

    } catch (const Error &err) {
        throw ErrorException(REDIS_SERVER_SIDE_FAILED);
    }
}

bool RedisIOClient::Remove(Data &source) {
    AUTO_TRACER("RedisIOClient::Remove", source);
    auto resp = m_redisCluster->del(source.id_.c_str());
    return true;
}

size_t RedisIOClient::Size(Data &source) {
    auto resp = m_redisCluster->get(source.id_.c_str());
    if(resp){
        return resp->size();
    }else return 0;
}
