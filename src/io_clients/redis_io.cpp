//
// Created by Jie on 8/25/20.
//

#include "redis_io.h"
#include <string.h>

RedisIOClient::RedisIOClient() {
    ConnectionOptions connectionOptions;
    connectionOptions.host = 127.0.0.1; // redis_cluster ip
    connectionOptions.port = 6379; // redis_cluster port

    m_redisCluster = std::make_shared<RedisCluster>(connectionOptions);
}

void RedisIOClient::Read(Data &source, Data &destination) {
    try {
        std::string key = std::string(source.id_.c_str());
        auto resp = m_redisCluster.get(key);
        if(!resp){
            // return errcode
        }

        memcpy(destination.buffer_, (*resp).c_str(), (*resp).length());
        destination.data_size_ = (*resp).length();

        // return successful
    } catch (const Error &err){
        //return errcode
    }
}

void RedisIOClient::Write(Data &source, Data &destination) {
    try {
        std::string key = std::string(source.id_.c_str());
        std::string value = std::string((char*)source.buffer_);
        bool result = m_redisCluster.set(key, value);
        if(!result){
            // return errcode
        }
        // return successful
    } catch (const Error &err){
        // return errcode
    }
}

