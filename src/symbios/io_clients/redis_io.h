//
// Created by Jie on 8/25/20.
// It is the implementation of Redis IO
//

#ifndef SYMBIOS_REDIS_IO_H
#define SYMBIOS_REDIS_IO_H

#include <symbios/io_clients/io.h>
#include <sw/redis++/redis++.h>

using namespace sw::redis;

class RedisIOClient: public IOClient {
public:
    /*
     * Constructor
     */
    RedisIOClient(uint16_t storage_index):IOClient(storage_index){
        auto redis_solution = std::static_pointer_cast<RedisSS>(solution);

        ConnectionOptions connectionOptions;
        connectionOptions.host = redis_solution->end_point_.c_str(); // redis_cluster ip
        connectionOptions.port = redis_solution->port_; // redis_cluster port

        m_redisCluster = std::make_shared<RedisCluster>(connectionOptions);
    }
    /*
     * Methods
     */

    /*
     * Read data from source into destination buffer while respecting the position_
     */
    void Read(Data &source, Data &destination) override;

    void Remove(Data &source) override;

    /*
     * Write data from source into destination buffer while respecting the position_
     */
    void Write(Data &source, Data &destination) override;

    size_t Size(Data &source) override;

private:
    std::shared_ptr<RedisCluster>   m_redisCluster;
};

#endif //SYMBIOS_REDIS_IO_H
