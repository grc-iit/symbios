//
// Created by lukemartinlogan on 8/26/20.
//

#ifndef SYMBIOS_FILE_H
#define SYMBIOS_FILE_H

#include <memory>

class File {
public:
    File() = default;
    virtual void Read(void *buffer, size_t size) = 0;
    virtual void Write(void *buffer, size_t size) = 0;
    virtual void Seek(size_t off) = 0;
    virtual void Pread(void *buffer, size_t size, size_t off) = 0;
    virtual void Pwrite(void *buffer, size_t size, size_t off) = 0;
    virtual void Close(void) = 0;
};

typedef std::unique_ptr<File> FilePtr;

#endif //SYMBIOS_FILE_H
