//
// Created by lukemartinlogan on 8/25/20.
//

#ifndef SYMBIOS_ORANGEFS_H
#define SYMBIOS_ORANGEFS_H

#define _GNU_SRC
#include <iostream>
#include <string>
#include <memory>
#include <filesystem>
#include <unistd.h>
#include <fcntl.h>

#include <sys/stat.h>

#include <cstdio>
#include <benchmark/io_client.h>
#include <benchmark/file.h>

class OrangefsFile : public File {
private:
    std::string path_;
    int fp_ = -1;

public:
    OrangefsFile(std::string path, int mode) : path_(std::move(path)) {
        int flags = 0;
        if((mode & FileMode::kRead) && (mode & FileMode::kWrite)) { flags |= O_RDWR; }
        else if(mode & FileMode::kRead) { flags |= O_RDONLY; }
        else if(mode & FileMode::kWrite) { flags |= O_WRONLY; }
        if(mode & FileMode::kCreate) { flags |= O_CREAT; }
        if(mode & FileMode::kDirect) { flags |= O_DIRECT; }
        if(mode & FileMode::kSync) { flags |= O_SYNC; }

        fp_ = open(path_.c_str(), flags);
        if(fp_ < 0) {
            std::cout << "Could not open file: " << path_ << std::endl;
            perror("open orangefs");
            throw 1;
        }
    }

    ~OrangefsFile() {
        close(fp_);
    };

    void Read(void *buffer, size_t size) {
        int ret = read(fp_, buffer, size);
        if(ret < 0) {
            std::cout << "Failed to read data" << std::endl;
            throw 1;
        }
    }

    void Write(void *buffer, size_t size) {
        int ret = write(fp_, buffer, size);
        if(ret < 0) {
            std::cout << "Failed to write data" << std::endl;
            throw 1;
        }
    }

    void Seek(size_t off) {
        lseek(fp_, off, SEEK_SET);
    }
};

class OrangefsIO : public IOClient {
private:
    std::string addr_;
    int port_ = -1;

public:
    OrangefsIO() = default;

    void Connect(std::string addr, int port) {
        addr_ = addr;
    }

    FilePtr Open(std::string path, int mode) {
        return std::make_unique<OrangefsFile>(addr_ + path, mode);
    }

    void Mkdir(std::string path) {
        std::filesystem::create_directory(path);
    }

    void Rmdir(std::string path) {
        std::filesystem::remove_all(path);
    }

    void Remove(std::string path) {
        std::filesystem::remove_all(path);
    }

    DirectoryListPtr Ls(std::string path) {
        DirectoryListPtr entries = std::make_unique<std::list<std::string>>();
        for(auto &p : std::filesystem::directory_iterator(path)) {
            entries->emplace_back(p.path());
        }
        return std::move(entries);
    }
};



#endif //SYMBIOS_ORANGEFS_H
