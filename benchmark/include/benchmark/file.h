//
// Created by lukemartinlogan on 8/26/20.
//

#ifndef SYMBIOS_FILE_H
#define SYMBIOS_FILE_H

#include <memory>
#include <list>
#include <string>

class File;
typedef std::unique_ptr<File> FilePtr;

class File {
public:
    File() = default;
    virtual void Read(void *buffer, size_t size) = 0;
    virtual void Write(void *buffer, size_t size) = 0;
    virtual void Seek(size_t off) = 0;
};

#endif //SYMBIOS_FILE_H
