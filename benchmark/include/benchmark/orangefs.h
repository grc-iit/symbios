//
// Created by lukemartinlogan on 8/25/20.
//

#ifndef SYMBIOS_ORANGEFS_H
#define SYMBIOS_ORANGEFS_H

#include <iostream>
#include <string>
#include <memory>

#include <sys/stat.h>

#include <benchmark/io_client.h>
#include <benchmark/file.h>

class OrangefsIO : public IOClient, public File {
private:
    std::string addr_;
    int port_ = -1;

public:
    OrangefsIO() = default;

    void Connect(std::string addr, int port) {
        addr_ = addr;
    }

    FilePtr Open(std::string path, std::string mode) {
        return std::make_unique<OrangefsIO>();
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



#endif //SYMBIOS_ORANGEFS_H
