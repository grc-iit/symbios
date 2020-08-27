//
// Created by lukemartinlogan on 8/25/20.
//

#ifndef SYMBIOS_IO_CLIENT_H
#define SYMBIOS_IO_CLIENT_H

#include <benchmark/file.h>
#include <string>
#include <memory>

class IOClient {
public:
    IOClient() = default;
    virtual void connect(std::string addr, int port) = 0;
    virtual FilePtr open(std::string path, std::string mode) = 0;
    virtual void mkdir(std::string path) = 0;
    virtual void rmdir(std::string path) = 0;
    virtual void remove(std::string path) = 0;
    virtual void ls(std::string path) = 0;
    //virtual void stat() = 0;

    virtual void add_key(std::string key, std::string value) = 0;
    //virtual bool has_key(std::string key) = 0;
    virtual std::string get_key(std::string key) = 0;
    virtual void rm_key(std::string key) = 0;
};

typedef std::unique_ptr<IOClient> IOClientPtr;

#endif //SYMBIOS_IO_CLIENT_H
