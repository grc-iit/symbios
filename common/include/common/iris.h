//
// Created by neeraj on 9/1/20.
//

#ifndef SYMBIOS_IRIS_H
#define SYMBIOS_IRIS_H

#include <symbios/io_clients/io_factory.h>
#include <symbios/common/enumerations.h>
#include <symbios/common/error_codes.h>
#include <symbios/client/client.h>
#include <common/debug.h>
#include <vector>
#include <mpi.h>


void slice_str(const char * str, char * buffer, size_t start, size_t end);


typedef enum IOLib {
    POSIX,
    IRIS,
    NIOBE,
    SYMBIOS
} IOLib;

typedef enum OPType {
    READ,
    WRITE,
    DELETE
} OPType;


typedef struct DataDescriptor {
    CharStruct id_;
    long position_; // read/write start position
    long size;
    uint chunk_index;
};


/*
 * Libhandler is to handle the different libraries mentioned in enum IOLib.
 *
 */

class LibHandler{
private:
    DataDescriptor src;
    std::vector<DataDescriptor> objs;
    uint16_t lib_type;
    uint16_t db_type;
    long max_obj_size;
    bool print_p;
    std::string file_;
    std::string symbios_conf;
    symbios::Client *symbios_client;
    std::vector<DataDescriptor> map_data();
    void do_mapped_read(long offset, size_t request_size, char *data);
    void do_mapped_write(long offset, size_t request_size, char *data);
    void do_mapped_delete(long offset, size_t request_size, char *data);
public:
    LibHandler(std::string file_, IOLib lib_type_, uint16_t io_type_, long max_obj_size_, bool print_p_, std::string symbios_conf_);
    ~LibHandler();
    void run(OPType op_type, long offset, size_t request_size, char *data);
};


class DataMapper {

public:
    uint16_t storage_type_;
    uint maxObjSize;

    DataMapper(uint16_t type_, uint maxObjSize_);
    std::vector<DataDescriptor> map(DataDescriptor &src);

};


class doOp{

public:
    std::shared_ptr<IOClient> client;
    IOFactory factory;
    std::string buffer_;
    doOp(int16_t type_);
    void Write(Data &source, Data &destination);
    void Read(Data &source, Data &destination);
    void Remove(Data &source);
    ~doOp();
};


#endif //SYMBIOS_IRIS_H
