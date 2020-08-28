//
// Created by Jie on 8/26/20.
// Define the data structures used in
//

#ifndef SYMBIOS_DATA_STRUCTURE_H
#define SYMBIOS_DATA_STRUCTURE_H

#include <basket/common/data_structures.h>
#include <symbios/common/enumerations.h>

typedef struct Data{
    CharStruct id_; // for file_io, the id_ is the filename; for Key-Value store io, the id_ is the key.
    long position_; // read or write start position
    void* buffer_;  // data
    long data_size_; // data size
    IOClientType native_io_client_type_;

    /*Define the default, copy and move constructor*/
    Data():id_(),position_(0),buffer_(),data_size_(0), native_io_client_type_(FILE_IO){}
    Data(const Data &other):id_(other.id_), position_(other.position_),buffer_(other.buffer_),
                            data_size_(other.data_size_), native_io_client_type_(other.native_io_client_type_) {}
    Data(Data &other): id_(other.id_), position_(other.position_), buffer_(other.buffer_),
                       data_size_(other.data_size_), native_io_client_type_(other.native_io_client_type_) {}

    /*Define Assignment Operator*/
    Data &operator=(const Data &other){
        id_ = other.id_;
        position_ = other.position_;
        memcpy(buffer_ + position_, other.buffer_ + other.position_, other.data_size_);
        data_size_ = other.data_size_;
        native_io_client_type_ = other.native_io_client_type_;

        return *this;
    }
} Data;

#endif //SYMBIOS_DATA_STRUCTURE_H

