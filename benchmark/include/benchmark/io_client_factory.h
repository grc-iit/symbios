//
// Created by lukemartinlogan on 8/25/20.
//

#ifndef SYMBIOS_IO_CLIENT_FACTORY_H
#define SYMBIOS_IO_CLIENT_FACTORY_H

#include <memory>

#include <benchmark/io_client.h>
#include <benchmark/mongo.h>
#include <benchmark/orangefs.h>
#include <benchmark/redis.h>

enum class IOClientType {
    kOrangefs, kMongo, kRedis
};

class IOClientFactory {
public:
    IOClientFactory() = default;
    static IOClientPtr Get(IOClientType type) {
        switch(type) {
            case IOClientType::kMongo:
                return std::make_shared<MongoIO>();
            case IOClientType::kOrangefs:
                return std::make_shared<OrangefsIO>();
            case IOClientType::kRedis:
                return std::make_shared<RedisIO>();
        }
    }
};


#endif //SYMBIOS_IO_CLIENT_FACTORY_H
