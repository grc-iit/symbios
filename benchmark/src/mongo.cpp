//
// Created by lukemartinlogan on 8/25/20.
//

//http://mongocxx.org/mongocxx-v3/tutorial/

#include <string>
#include <memory>

#include <benchmark/mongo.h>
#include <benchmark/io_client.h>
#include <benchmark/file.h>

class MongoFile : File {
public:
    MongoFile() {
        throw 1;
    }

    void read(void *buffer, size_t size) {
        throw 1;
    }

    void write(void *buffer, size_t size) {
        throw 1;
    }

    void seek(size_t off) {
        throw 1;
    }

    void pread(void *buffer, size_t size, size_t off) {
        throw 1;
    }

    void pwrite(void *buffer, size_t size, size_t off) {
        throw 1;
    }

    void close(void) {
        throw 1;
    }
};

class Mongo : IOClient {
private:
    std::string addr_;
    int port_ = -1;

public:
    Redis() = default;

    void connect(std::string addr, int port) {
        throw 1;
    }

    FilePtr open(std::string path, std::string mode) {
        return std::unique_ptr<MongoFile>(new MongoFile());
    }

    void mkdir(std::string path) {
        throw 1;
    }

    void rmdir(std::string path) {
        throw 1;
    }

    void remove(std::string path) {
        throw 1;
    }

    void ls(std::string path) {
        throw 1;
    }

    void add_key(std::string key, std::string value) {
        throw 1;
    }

    void rm_key(std::string key) {
        throw 1;
    }
};
