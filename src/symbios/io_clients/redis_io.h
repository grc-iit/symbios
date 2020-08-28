//
// Created by Jie on 8/25/20.
// It is the implementation of Redis IO
//

#ifndef SYMBIOS_REDIS_IO_H
#define SYMBIOS_REDIS_IO_H

#include <symbios/io_clients/object_store_io.h>
#include <redis-plus-plus/sw/redis++/redis++.h>

using namespace sw::redis

class ObjectStoreIOClient: public KeyValueStoreIO {
public:
    /*
     * Constructor
     */
    RedisIOClient()
    /*
     * Methods
     */

    /*
     * Read data from source into destination buffer while respecting the position_
     */
    void Read(Data &source, Data &destination) override;

    /*
     * Write data from source into destination buffer while respecting the position_
     */
    void Write(Data &source, Data &destination) override;

private:
    std::shared_ptr<RedisCluster>   m_redisCluster;
};

#endif //SYMBIOS_REDIS_IO_H
