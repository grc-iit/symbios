//
// Created by mani on 8/24/2020.
//

#include <common/debug.h>
#include <fcntl.h>
#include <string>
#include <symbios/common/error_codes.h>
#include <symbios/io_clients/file_io.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void FileIOClient::Read(Data &source, Data &destination) {
    auto tracer_source = common::debug::AutoTrace("FileIOClient::Read", source, destination);
    const char *file_name = source.id_.c_str();
    int fileFd = open(file_name, O_RDONLY);
    if (fileFd == -1) {
        throw ErrorException(OPEN_FILE_FAILED);
    } else {
        auto file_size = lseek(fileFd, 0L, SEEK_END);
        if (lseek(fileFd, source.position_, SEEK_SET) == -1) {
            close(fileFd);
            throw ErrorException(SEEK_FILE_FAILED);
        } else {
            auto source_data_size = source.buffer_.size();
            if (source.buffer_.size() == 0) {
                source_data_size = file_size - source.position_;
            }
            destination.buffer_.resize(source_data_size);
            size_t data_size = read(fileFd, destination.buffer_.data() + destination.position_, source_data_size);
            if (data_size == source_data_size) {
                // read data from file successful
                close(fileFd);
            } else {
                // read data from file failed.
                close(fileFd);
                throw ErrorException(READ_DATA_FROM_FILE_FAILED);
            }
        }
    }
    COMMON_DBGVAR(destination);
}

void FileIOClient::Write(Data &source, Data &destination) {
    auto tracer_source = common::debug::AutoTrace("FileIOClient::Read", source, destination);
    const char *dest_file_name = destination.id_.c_str();
    int fileFd = open(dest_file_name, O_RDWR | O_CREAT, 0644);
    if (fileFd == -1) {
        throw ErrorException(OPEN_FILE_FAILED);
    } else {
        if (lseek(fileFd, destination.position_, SEEK_SET) == -1) {
            close(fileFd);
            throw ErrorException(SEEK_FILE_FAILED);
        } else {
            ssize_t size = write(fileFd, source.buffer_.c_str() + source.position_,
                                 source.buffer_.size() - source.position_);
            COMMON_DBGVAR(size);
            if (size < source.buffer_.size() - source.position_) {
                // write data to file failed
                close(fileFd);
                throw ErrorException(WRITE_DATA_TO_FILE_FAILED);
            }
            close(fileFd);
        }
    }
}

bool FileIOClient::Remove(Data &source) {
    auto tracer_source = common::debug::AutoTrace("FileIOClient::Remove", source);
    remove(source.id_.c_str());
    return true;
}

size_t FileIOClient::Size(Data &source) {
    if(boost::filesystem::exists(source.id_.c_str())){
        return boost::filesystem::file_size(source.id_.c_str());
    }
    return 0;
}
