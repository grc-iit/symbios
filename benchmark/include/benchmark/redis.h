//
// Created by lukemartinlogan on 8/25/20.
//

//https://github.com/redis/hiredis
//https://github.com/sewenew/redis-plus-plus

#ifndef SYMBIOS_REDIS_H
#define SYMBIOS_REDIS_H

#include <string>
#include <memory>
#include <cstring>
#include <utility>
#include <sw/redis++/redis++.h>

#include <benchmark/io_client.h>
#include <benchmark/file.h>

class RedisIO : public IOClient, public File {
private:
    static const size_t kBlockSize = (1<<20);
    size_t off_ = 0;
    std::string addr_;
    int port_ = -1;
    std::shared_ptr<sw::redis::Redis> context_;

    void SetKey(std::string key, std::string value="") {
        throw 1;
    }

    bool HasKey(std::string key) {
        return false;
    }

    void GetKey(std::string key, std::string &value) {
        throw 1;
    }

    void RemoveKey(std::string key) {
        throw 1;
    }

public:
    RedisIO() = default;
    RedisIO(std::string path, std::shared_ptr<sw::redis::Redis> context) : addr_(std::move(path)), context_(std::move(context)) {}

    void Connect(std::string addr, int port) {
        /*ConnectionOptions connectionOptions;
        connectionOptions.host = addr; // redis_cluster ip
        connectionOptions.port = port; // redis_cluster port

        context_ = std::make_shared<RedisCluster>(connectionOptions);*/
    }

    FilePtr Open(std::string path, std::string mode) {
        if(!HasKey(path)) {
            SetKey(path);
        }
        return std::make_unique<RedisIO>(path, context_);
    }

    void Mkdir(std::string path) {
        SetKey(path);
    }

    void Rmdir(std::string path) {
        RemoveKey(path);
    }

    void Remove(std::string path) {
        RemoveKey(path);
    }

    DirectoryListPtr Ls(std::string path) {
        throw 1;
    }

    void Read(void *buffer, size_t size) {
        size_t suffix = off_/kBlockSize;
        size_t rem = off_ % kBlockSize;
        std::string path = addr_ + std::to_string(suffix);
        std::string block(kBlockSize, 0);
        for(size_t i = 0; i < size; i += kBlockSize) {
            std::string path = addr_ + std::to_string(suffix++);
            size_t buf_size = (i+kBlockSize)<size ? kBlockSize : size - i;
            GetKey(path, block);
            memcpy((char*)buffer + i, block.c_str() + rem, buf_size);
            rem = 0;
        }
        off_ += size;
    }

    void Write(void *buffer, size_t size) {
        size_t suffix = off_/kBlockSize;
        size_t rem = off_ % kBlockSize;
        std::string path = addr_ + std::to_string(suffix);
        std::string block(kBlockSize, 0);
        for(size_t i = 0; i < size; i += kBlockSize) {
            std::string path = addr_ + std::to_string(suffix++);
            size_t buf_size = (i+kBlockSize)<size ? kBlockSize : size - i;
            memcpy((void *) (block.c_str() + rem), (char*)buffer + i, buf_size);
            GetKey(path, block);
            rem = 0;
        }
        off_ += size;
    }

    void Seek(size_t off) {
        off_ = off;
    }

    void Close(void) {}

};

#endif //SYMBIOS_REDIS_H
