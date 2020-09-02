//
// Created by neeraj on 9/1/20.
//

#ifndef SYMBIOS_IRIS_H
#define SYMBIOS_IRIS_H

#include <symbios/io_clients/io_factory.h>
#include <symbios/common/enumerations.h>
#include <symbios/common/error_codes.h>
#include <common/debug.h>
#include <vector>

typedef struct DataDescriptor {
    CharStruct id_;
    size_t position_; // read/write start position
    uint16_t size;
    uint chunk_index;
};

class DataMapper {

public:
    uint16_t storage_type_;
    uint maxObjSize;

    DataMapper(uint16_t type_, uint maxObjSize_);
    std::vector<DataDescriptor> generateDataObjects(DataDescriptor &src);

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
