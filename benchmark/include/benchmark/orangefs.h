//
// Created by lukemartinlogan on 8/25/20.
//

#ifndef SYMBIOS_ORANGEFS_H
#define SYMBIOS_ORANGEFS_H

#include <iostream>
#include <string>
#include <memory>
#include <filesystem>

#include <sys/stat.h>

#include <cstdio>
#include <benchmark/io_client.h>
#include <benchmark/file.h>

class OrangefsIO : public IOClient, public File {
private:
    std::FILE *fp_ = nullptr;
    std::string addr_;
    int port_ = -1;

public:
    OrangefsIO() = default;
    OrangefsIO(std::string path, std::string mode) {
        fp_ = std::fopen(path.c_str(), mode.c_str());
        if(fp_ == nullptr) {
            throw 1;
        }
    }

    void Connect(std::string addr, int port) {
        addr_ = addr;
    }

    FilePtr Open(std::string path, std::string mode) {
        return std::make_unique<OrangefsIO>(addr_ + path, mode);
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

    void Read(void *buffer, size_t size) {
        std::fread(buffer, 1, size, fp_);
    }

    void Write(void *buffer, size_t size) {
        std::fwrite(buffer, 1, size, fp_);
    }

    void Seek(size_t off) {
        std::fseek(fp_, off, SEEK_SET);
    }

    void Close(void) {
        std::fclose(fp_);
    }
};



#endif //SYMBIOS_ORANGEFS_H
