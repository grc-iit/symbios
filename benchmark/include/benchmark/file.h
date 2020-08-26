//
// Created by lukemartinlogan on 8/26/20.
//

#ifndef SYMBIOS_FILE_H
#define SYMBIOS_FILE_H

#include <memory>

class File {
public:
    File() = default;
    virtual void read(void *buffer, size_t size) = 0;
    virtual void write(void *buffer, size_t size) = 0;
    virtual void seek(size_t off) = 0;
    virtual void pread(void *buffer, size_t size, size_t off) = 0;
    virtual void pwrite(void *buffer, size_t size, size_t off) = 0;
    virtual void close(void) = 0;
};

typedef std::unique_ptr<File> FilePtr;

#endif //SYMBIOS_FILE_H
