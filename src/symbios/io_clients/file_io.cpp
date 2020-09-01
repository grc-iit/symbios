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
  auto tracer_source =
      common::debug::AutoTrace(std::string("FileIOClient::Read"), source);
  auto tracer_destination =
      common::debug::AutoTrace(std::string("FileIOClient::Read"), destination);
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
      if (source.data_size_ == 0) {
        source.data_size_ = file_size - source.position_;
      }
      destination.buffer_ = malloc(source.data_size_);
      ssize_t data_size =
          read(fileFd, destination.buffer_ + destination.position_,
               source.data_size_);
      if (data_size == source.data_size_) {
        // read data from file successful
        destination.data_size_ = data_size;
        close(fileFd);
      } else {
        // read data from file failed.
        close(fileFd);
        throw ErrorException(READ_DATA_FROM_FILE_FAILED);
      }
    }
  }
}

void FileIOClient::Write(Data &source, Data &destination) {
  auto tracer_source =
      common::debug::AutoTrace(std::string("FileIOClient::Read"), source);
  auto tracer_destination =
      common::debug::AutoTrace(std::string("FileIOClient::Read"), destination);
  const char *dest_file_name = destination.id_.c_str();
  int fileFd = open(dest_file_name, O_RDWR | O_CREAT | O_APPEND, 0644);
  if (fileFd == -1) {
    throw ErrorException(OPEN_FILE_FAILED);
  } else {
    if (lseek(fileFd, destination.position_, SEEK_SET) == -1) {
      close(fileFd);
      throw ErrorException(SEEK_FILE_FAILED);
    } else {
      ssize_t size =
          write(fileFd, source.buffer_ + source.position_, source.data_size_);
      if (size < source.data_size_) {
        // write data to file failed
        close(fileFd);
        throw ErrorException(WRITE_DATA_TO_FILE_FAILED);
      }
      close(fileFd);
    }
  }
}

void FileIOClient::Remove(Data &source) {}
