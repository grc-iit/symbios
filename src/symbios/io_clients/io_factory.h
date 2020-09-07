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
#include <common/debug.h>

class IOFactory {
public:
    IOFactory(){
        for (auto entry : SYMBIOS_CONF->STORAGE_SOLUTIONS) {
            switch (entry.second->io_client_type_){
                case IOClientType::FILE_IO:
                    basket::Singleton<FileIOClient>::GetInstance(entry.first);
                    break;
                case IOClientType::MONGO_IO:
                    basket::Singleton<MongoIOClient>::GetInstance(entry.first);
                    break;
                case IOClientType::REDIS_IO:
                    basket::Singleton<RedisIOClient>::GetInstance(entry.first);
                    break;
            }
        }
    }

    std::shared_ptr<IOClient> GetIOClient(uint16_t storage_index){
        AUTO_TRACER("IOFactory::GetIOClient", storage_index);
        auto solution = SYMBIOS_CONF->STORAGE_SOLUTIONS[storage_index];
        switch (solution->io_client_type_){
            case IOClientType::FILE_IO:
                return basket::Singleton<FileIOClient>::GetInstance(storage_index);
            case IOClientType::MONGO_IO:
                return basket::Singleton<MongoIOClient>::GetInstance(storage_index);
            case IOClientType::REDIS_IO:
                return basket::Singleton<RedisIOClient>::GetInstance(storage_index);
        }
    }
};
#endif //SYMBIOS_IO_FACTORY_H
