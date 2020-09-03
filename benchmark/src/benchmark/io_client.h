//
// Created by lukemartinlogan on 8/25/20.
//

#ifndef SYMBIOS_IO_CLIENT_H
#define SYMBIOS_IO_CLIENT_H

#include <benchmark/file.h>
#include <string>
#include <memory>

class FileMode {
public:
    static const int kRead=(1<<0), kWrite=(1<<1), kCreate=(1<<2), kDirect=(1<<3), kSync=(1<<4);
};

class IOClient;
typedef std::unique_ptr<IOClient> IOClientPtr;
typedef std::unique_ptr<std::list<std::string>> DirectoryListPtr;

class IOClient {
public:
    IOClient() = default;
    virtual ~IOClient() = default;
    virtual void Connect(std::string addr, int port) = 0;
    virtual FilePtr Open(std::string path, int mode) = 0;
    virtual void Mkdir(std::string path) = 0;
    virtual void Rmdir(std::string path) = 0;
    virtual void Remove(std::string path) = 0;
    virtual DirectoryListPtr Ls(std::string path) = 0;
    //virtual void Stat() = 0;
};

#endif //SYMBIOS_IO_CLIENT_H
