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
    FOPEN,
    LSEEK,
    FCLOSE
} OPType;


typedef struct DataDescriptor {
    CharStruct id_;
    size_t position_; // read/write start position
    size_t size;
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
    uint16_t max_obj_size;
    std::string file_;
    std::vector<DataDescriptor> map_data();
    void do_mapped_read(size_t offset, size_t request_size, char *data);
    void do_mapped_write(size_t offset, size_t request_size, char *data);
public:
    LibHandler(std::string file_, IOLib lib_type_, uint16_t io_type_, uint16_t max_obj_size_);
    void run(OPType op_type, size_t offset, size_t request_size, char *data);
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
    ~doOp();
};


#endif //SYMBIOS_IRIS_H
