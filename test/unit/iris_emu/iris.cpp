//
// Created by neeraj on 9/1/20.
//

#include "iris.h"


doOp::doOp(int16_t type_) {
    factory = IOFactory();
    client = factory.GetIOClient(type_);
}

void doOp::Write(Data &source, Data &destination) {
    client->Write(source, destination);
}

void doOp::Read(Data &source, Data &destination) {
    client->Read(source, destination);
}

doOp::~doOp() {}

DataMapper::DataMapper(uint16_t type_, uint maxObjSize_) {
    storage_type_ = type_;
    maxObjSize = maxObjSize_;
}

std::vector<DataDescriptor> DataMapper::generateDataObjects(DataDescriptor &src){
    std::vector<DataDescriptor> objects;

    std::size_t remainingOperationSize = src.size;
    int curr_pos = src.position_;
    while(remainingOperationSize!=0) {
        DataDescriptor obj;
        auto index = curr_pos / maxObjSize;
        obj.id_=src.id_+ std::to_string(index);
        obj.position_ = curr_pos % maxObjSize ;
        obj.size = remainingOperationSize > maxObjSize - obj.position_ ? maxObjSize - obj.position_ : remainingOperationSize;
        remainingOperationSize -= obj.size;
        curr_pos += obj.size;
        obj.chunk_index = index;
        objects.push_back(obj);
    }

    return objects;
}
