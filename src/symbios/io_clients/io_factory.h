//
// Created by mani on 8/24/2020.
// Defines the Factory for choosing various IO Clients
//

#ifndef SYMBIOS_IO_FACTORY_H
#define SYMBIOS_IO_FACTORY_H

#include <basket/common/singleton.h>
#include <symbios/common/enumerations.h>
#include <symbios/io_clients/io.h>
#include <symbios/io_clients/file_io.h>
#include <symbios/io_clients/mongo_io.h>
#include <symbios/io_clients/redis_io.h>

class IOFactory {
public:
    IOFactory(){}

    std::shared_ptr<IOClient> GetIOClient(IOClientType &type){
        switch (type){
            case IOClientType::FILE_IO:
                return basket::Singleton<FileIOClient>::GetInstance();
            case IOClientType::MONGO_IO:
                return basket::Singleton<MongoIOClient>::GetInstance();
            case IOClientType::REDIS_IO:
                return basket::Singleton<RedisIOClient>::GetInstance();
        }
    }
};
#endif //SYMBIOS_IO_FACTORY_H
