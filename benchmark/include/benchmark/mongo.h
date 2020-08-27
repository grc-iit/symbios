//
// Created by lukemartinlogan on 8/25/20.
//

//http://mongocxx.org/mongocxx-v3/tutorial/
//http://mongocxx.org/mongocxx-v3/installation/

#ifndef SYMBIOS_MONGO_H
#define SYMBIOS_MONGO_H

#include <string>
#include <memory>
#include <cstring>

#include <benchmark/io_client.h>
#include <benchmark/file.h>
#include <benchmark/kvs.h>

class MongoContext : public KVS {
public:
    MongoContext(std::string addr, int port) {
        throw 1;
    }

    void SetKey(std::string key, std::string value="") {
        throw 1;
    }

    bool HasKey(std::string key) {
        throw 1;
    }

    void GetKey(std::string key, std::string &value) {
        throw 1;
    }

    void RemoveKey(std::string key) {
        throw 1;
    }
};

class MongoFile : public File {
private:
    static const size_t kBlockSize = (1<<20);
    std::string path_;
    size_t off_ = 0;
    std::shared_ptr<MongoContext> context_;

public:
    MongoFile(std::shared_ptr<MongoContext> context, std::string path) : context_(context), path_(std::move(path)) {}

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

class MongoIO : public IOClient {
private:
    std::string addr_;
    int port_ = -1;
    std::shared_ptr<MongoContext> context_;

public:
    MongoIO() = default;

    void Connect(std::string addr, int port) {
        context_ = std::make_shared<MongoContext>(addr, port);
    }

    FilePtr Open(std::string path, std::string mode) {
        if(!context_->HasKey(path)) {
            context_->SetKey(path);
        }
        return std::make_unique<MongoFile>(context_, path);
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

#endif //SYMBIOS_MONGO_H
