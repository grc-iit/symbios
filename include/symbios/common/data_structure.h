//
// Created by Jie on 8/26/20.
// Define the data structures used in
//

#ifndef SYMBIOS_DATA_STRUCTURE_H
#define SYMBIOS_DATA_STRUCTURE_H

#include <basket/common/data_structures.h>
#include <symbios/common/enumerations.h>
#include <rpc/msgpack.hpp>

typedef struct Data{
    CharStruct id_; // for file io, the "id_" is the filename; for object store io, the "id_" is the key.
    long position_; // read/write start position
    void* buffer_;  // data content
    long data_size_; // data size need to read/write
    IOClientType io_client_type_; // io client type

    /*Define the default, copy and move constructor*/
    Data(): id_(), position_(0), buffer_(), data_size_(0), io_client_type_(FILE_IO){}
    Data(const Data &other): id_(other.id_), position_(other.position_), buffer_(other.buffer_),
                             data_size_(other.data_size_), io_client_type_(other.io_client_type_) {}
    Data(Data &other): id_(other.id_), position_(other.position_), buffer_(other.buffer_),
                       data_size_(other.data_size_), io_client_type_(other.io_client_type_) {}

    /*Define Assignment Operator*/
    Data &operator=(const Data &other){
        id_ = other.id_;
        position_ = other.position_;
        memcpy(buffer_ + position_, other.buffer_ + other.position_, other.data_size_);
        data_size_ = other.data_size_;
        io_client_type_ = other.io_client_type_;

        return *this;
    }
} Data;

typedef struct StorageSolution{
    CharStruct end_point_;
    IOClientType io_client_type_;
    /*Define the default, copy and move constructor*/
    StorageSolution(): end_point_(), io_client_type_(){}
    StorageSolution(CharStruct end_point, IOClientType io_client_type): end_point_(end_point), io_client_type_(io_client_type){}
    StorageSolution(const StorageSolution &other): end_point_(other.end_point_),
                                                   io_client_type_(other.io_client_type_){}
    StorageSolution(StorageSolution &other): end_point_(other.end_point_),
                                             io_client_type_(other.io_client_type_){}

    /*Define Assignment Operator*/
    StorageSolution &operator=(const StorageSolution &other){
        end_point_ = other.end_point_;
        io_client_type_ = other.io_client_type_;
        return *this;
    }
} StorageSolution;

typedef struct FileSS: public StorageSolution{
    /*Define the default, copy and move constructor*/
    FileSS(CharStruct end_point): StorageSolution(end_point,IOClientType::FILE_IO){}
    FileSS(const FileSS &other): StorageSolution(other){}
    FileSS(FileSS &other): StorageSolution(other){}
    /*Define Assignment Operator*/
    FileSS &operator=(const FileSS &other){
        StorageSolution::operator=(other);
        return *this;
    }
}FileStorageSolution;

typedef struct RedisSS: public StorageSolution{
    uint16_t port_;
    /*Define the default, copy and move constructor*/
    RedisSS(CharStruct end_point,  uint16_t port):StorageSolution(end_point,IOClientType::REDIS_IO),port_(port){}
    RedisSS(const RedisSS &other):StorageSolution(other),port_(other.port_){}
    RedisSS(RedisSS &other):StorageSolution(other),port_(other.port_){}
    /*Define Assignment Operator*/
    RedisSS &operator=(const RedisSS &other){
        StorageSolution::operator=(other);
        port_=other.port_;
        return *this;
    }
}RedisSS;

typedef struct MongoSS: public StorageSolution{
    CharStruct database_;
    CharStruct collection_;
    /*Define the default, copy and move constructor*/
    MongoSS(CharStruct end_point,CharStruct database,CharStruct collection):StorageSolution(end_point,IOClientType::MONGO_IO),database_(database),collection_(collection){}
    MongoSS(const MongoSS &other):StorageSolution(other),database_(other.database_),collection_(other.collection_){}
    MongoSS(MongoSS &other):StorageSolution(other),database_(other.database_),collection_(other.collection_){}
    /*Define Assignment Operator*/
    MongoSS &operator=(const MongoSS &other){
        StorageSolution::operator=(other);
        database_=other.database_;
        collection_=other.collection_;
        return *this;
    }
}MongoSS;


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



#include <rpc/msgpack.hpp>
namespace clmdep_msgpack {
    MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
        namespace adaptor {
            namespace mv1 = clmdep_msgpack::v1;
            template<>
            struct convert<Data> {
                mv1::object const &operator()(mv1::object const &o, Data &input) const {
                    input.id_ = o.via.array.ptr[0].as<CharStruct>();
                    input.position_ = o.via.array.ptr[1].as<size_t>();
                    input.buffer_ = (void*)o.via.array.ptr[2].as<std::string>().c_str();
                    input.data_size_ = o.via.array.ptr[3].as<size_t>();
                    return o;
                }
            };

            template<>
            struct pack<Data> {
                template<typename Stream>
                packer <Stream> &operator()(mv1::packer <Stream> &o, Data const &input) const {
                    o.pack_array(4);
                    o.pack(input.id_);
                    o.pack(input.position_);
                    o.pack(std::string((char*)input.buffer_));
                    o.pack(input.data_size_);
                    return o;
                }
            };

            template<>
            struct object_with_zone<Data> {
                void operator()(mv1::object::with_zone &o, Data const &input) const {
                    o.type = type::ARRAY;
                    o.via.array.size = 4;
                    o.via.array.ptr = static_cast<clmdep_msgpack::object *>(o.zone.allocate_align(
                            sizeof(mv1::object) * o.via.array.size, MSGPACK_ZONE_ALIGNOF(mv1::object)));
                    o.via.array.ptr[0] = mv1::object(input.id_, o.zone);
                    o.via.array.ptr[1] = mv1::object(input.position_, o.zone);
                    o.via.array.ptr[2] = mv1::object(std::string((char*)input.buffer_), o.zone);
                    o.via.array.ptr[3] = mv1::object(input.data_size_, o.zone);
                }
            };
        }  // namespace adaptor
    }
}  // namespace clmdep_msgpack
std::ostream &operator<<(std::ostream &os, Data &data);

#endif //SYMBIOS_DATA_STRUCTURE_H

