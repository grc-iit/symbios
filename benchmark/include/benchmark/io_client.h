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
    virtual void Connect(std::string addr, int port) = 0;
    virtual FilePtr Open(std::string path, std::string mode) = 0;
    virtual void Mkdir(std::string path) = 0;
    virtual void Rmdir(std::string path) = 0;
    virtual void Remove(std::string path) = 0;
    virtual void Ls(std::string path) = 0;
    //virtual void Stat() = 0;

    virtual void AddKey(std::string key, std::string value) = 0;
    //virtual bool HasKey(std::string key) = 0;
    virtual std::string GetKey(std::string key) = 0;
    virtual void RemoveKey(std::string key) = 0;
};

typedef std::shared_ptr<IOClient> IOClientPtr;

#endif //SYMBIOS_IO_CLIENT_H
