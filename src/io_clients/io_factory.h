//
// Created by mani on 8/24/2020.
// Defines the Factory for choosing various IO Clients
//

#ifndef SYMBIOS_IO_FACTORY_H
#define SYMBIOS_IO_FACTORY_H

#include "singleton.h"
#include "io.h"
#include "file_io.h"
#include "key_value_store_io.h"
#include "mongo_io.h"
#include "redis_io.h"

class IOFactory {
public:
    IOFactory(){
        Singleton<FileIOClient>::GetInstance();
        Singleton<MongoIOClient>::GetInstance();
        Singleton<RedisIOClient>::GetInstance();
    }

    std::shared_ptr<IOClient> GetIOClient(IOClientType &type){
        switch (type){
            case IOClientType::FILE_IO:
                return Singleton<FileIOClient>::GetInstance();
            case IOClientType::MONGO_IO:
                return Singleton<MongoIOClient>::GetInstance();
            case IOClientType::REDIS_IO:
                return Singleton<RedisIOClient>::GetInstance();
        }
    }
};
#endif //SYMBIOS_IO_FACTORY_H
