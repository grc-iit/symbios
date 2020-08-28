//
// Created by Jie on 8/26/20.
// Define the data structures used in
//

#ifndef SYMBIOS_DATA_STRUCTURE_H
#define SYMBIOS_DATA_STRUCTURE_H

#include <basket/common/data_structures.h>
#include <symbios/common/enumerations.h>

typedef struct Data{
    CharStruct id_; // for file io, the "id_" is the filename; for object store io, the "id_" is the key.
    long position_; // read/write start position
    void* buffer_;  // data content
    long data_size_; // data size need to read/write
    IOClientType native_io_client_type_; // io client type

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


typedef struct Distribution{
    Data source_data_; // memory buffer for write and file buffer for read
    Data destination_data_; // file info for write and memory info for read
    IOClientType io_client_type_; // native io client type

    /*Define the default, copy and move constructor*/
    Distribution():io_client_type_(FILE_IO){}
    Distribution(const Distribution &other):source_data_(other.source_data_),
        destination_data_(other.destination_data_), io_client_type_(other.io_client_type_){}
    Distribution(Distribution &other):source_data_(other.source_data_),
        destination_data_(other.destination_data_), io_client_type_(other.io_client_type_){}

    /*Define Assignment Operator*/
    Distribution &operator=(const Distribution &other){
        source_data_ = other.source_data_;
        destination_data_ = other.destination_data_;
        io_client_type_ = other.io_client_type_;

        return *this;
    }

} Distribution;
#endif //SYMBIOS_DATA_STRUCTURE_H

