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
                return std::unique_ptr<Mongo>(new Mongo());
            case IOClientType::kOrangefs:
                return std::unique_ptr<Orangefs>(new Orangefs());
            case IOClientType::kRedis:
                return std::unique_ptr<Redis>(new Redis());
        }
    }
};


#endif //SYMBIOS_IO_CLIENT_FACTORY_H
