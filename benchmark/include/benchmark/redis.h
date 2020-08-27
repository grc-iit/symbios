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
#include <benchmark/kvs.h>

class RedisContext : public KVS {
private:
    std::unique_ptr<sw::redis::RedisCluster> context_;

public:
    RedisContext(std::string addr, int port) {
        sw::redis::ConnectionOptions connectionOptions;
        connectionOptions.host = addr; // redis_cluster ip
        connectionOptions.port = port; // redis_cluster port
        context_ = std::make_unique<sw::redis::RedisCluster>(connectionOptions);
    }

    void SetKey(std::string key, std::string value="") {
        context_->set(key, value);
    }

    bool HasKey(std::string key) {
        auto check = context_->get(key);
        if(check) { return true; }
        return false;
    }

    void GetKey(std::string key, std::string &value) {
        value = std::move(*(context_->get(key)));
    }

    void RemoveKey(std::string key) {
        context_->del(key);
    }
};

class RedisFile : public File {
private:
    static const size_t kBlockSize = (1<<20);
    std::string path_;
    size_t off_ = 0;
    std::shared_ptr<RedisContext> context_;

public:
    RedisFile(std::shared_ptr<RedisContext> context, std::string path) : context_(context), path_(std::move(path)) {}

    void Read(void *buffer, size_t size) {
        size_t suffix = off_/kBlockSize;
        size_t rem = off_ % kBlockSize;
        std::string path = path_ + std::to_string(suffix);
        std::string block;
        for(size_t i = 0; i < size; i += kBlockSize) {
            std::string path = path_ + std::to_string(suffix++);
            size_t buf_size = (i+kBlockSize)<size ? kBlockSize : size - i;
            context_->GetKey(path, block);
            memcpy((char*)buffer + i, block.c_str() + rem, buf_size);
            rem = 0;
        }
        off_ += size;
    }

    void Write(void *buffer, size_t size) {
        size_t suffix = off_/kBlockSize;
        size_t rem = off_ % kBlockSize;
        std::string path = path_ + std::to_string(suffix);
        std::string block;
        for(size_t i = 0; i < size; i += kBlockSize) {
            std::string path = path_ + std::to_string(suffix++);
            size_t buf_size = (i+kBlockSize)<size ? kBlockSize : size - i;
            memcpy((void *) (block.c_str() + rem), (char*)buffer + i, buf_size);
            context_->SetKey(path, block);
            rem = 0;
        }
        off_ += size;
    }

    void Seek(size_t off) {
        off_ = off;
    }

    void Close(void) {}
};

class RedisIO : public IOClient {
private:
    std::string addr_;
    int port_ = -1;
    std::shared_ptr<RedisContext> context_;

public:
    RedisIO() = default;

    void Connect(std::string addr, int port) {
        context_ = std::make_shared<RedisContext>(addr, port);
    }

    FilePtr Open(std::string path, std::string mode) {
        if(!context_->HasKey(path)) {
            context_->SetKey(path);
        }
        return std::make_unique<RedisFile>(context_, path);
    }

    void Mkdir(std::string path) {
        context_->SetKey(path);
    }

    void Rmdir(std::string path) {
        context_->RemoveKey(path);
    }

    void Remove(std::string path) {
        context_->RemoveKey(path);
    }

    DirectoryListPtr Ls(std::string path) {
        throw 1;
    }
};

#endif //SYMBIOS_REDIS_H
