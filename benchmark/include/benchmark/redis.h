//
// Created by lukemartinlogan on 8/25/20.
//

//https://github.com/redis/hiredis
//https://github.com/sewenew/redis-plus-plus

#ifndef SYMBIOS_REDIS_H
#define SYMBIOS_REDIS_H

#include <string>
#include <memory>
#include <utility>
#include <sw/redis++/redis++.h>

#include <benchmark/io_client.h>
#include <benchmark/file.h>

class RedisIO : public IOClient, public File {
private:
    std::string addr_;
    int port_ = -1;
    std::shared_ptr<sw::redis::Redis> context_;

public:
    RedisIO() = default;
    RedisIO(std::shared_ptr<sw::redis::Redis> context) : context_(std::move(context)) {}

    void Connect(std::string addr, int port) {
        ConnectionOptions connectionOptions;
        connectionOptions.host = addr; // redis_cluster ip
        connectionOptions.port = port; // redis_cluster port

        context_ = std::make_shared<RedisCluster>(connectionOptions);
    }

    FilePtr Open(std::string path, std::string mode) {
        return std::make_unique<RedisIO>(context_);
    }

    void Mkdir(std::string path) {
        throw 1;
    }

    void Rmdir(std::string path) {
        throw 1;
    }

    void Remove(std::string path) {
        throw 1;
    }

    void Ls(std::string path) {
        throw 1;
    }

    void Read(void *buffer, size_t size) {
        throw 1;
    }

    void Write(void *buffer, size_t size) {
        throw 1;
    }

    void Seek(size_t off) {
        throw 1;
    }

    void Pread(void *buffer, size_t size, size_t off) {
        throw 1;
    }

    void Pwrite(void *buffer, size_t size, size_t off) {
        throw 1;
    }

    void Close(void) {
        throw 1;
    }

    void AddKey(std::string key, std::string value) {
        throw 1;
    }

    std::string GetKey(std::string key) {
        throw 1;
    }

    void RemoveKey(std::string key) {
        throw 1;
    }
};

#endif //SYMBIOS_REDIS_H
