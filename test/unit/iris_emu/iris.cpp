//
// Created by neeraj on 9/1/20.
//

#include "iris.h"


//// doOp which does operations emulating iris ////

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


//// LibHandler

void LibHandler::run(uint16_t op_type) {
    map_data();
    if (op_type == OPType::READ) do_mapped_read();
    else if (op_type == OPType::WRITE) do_mapped_write();
    else if (op_type == OPType::FCLOSE);
    else if (op_type == OPType::FOPEN);
    else if (op_type == OPType::LSEEK);

}

void LibHandler::do_mapped_write() {
    DataDescriptor src = {file_,0,  data.size(), 0 };
    DataDescriptor read_src = {file_, 0,  data.size(), 0 };
    DataMapper mapper_(db_type, max_obj_size);
    auto objs = mapper_.map(src);
    auto read_objs = mapper_.map(read_src);

    if (lib_type == IOLib::IRIS){
        doOp operation(db_type);
        for (auto &i : objs){
            auto data_obj = Data();
            data_obj.id_= i.id_;
            data_obj.position_=i.position_;
            data_obj.buffer_= data.substr(i.position_+i.chunk_index*max_obj_size, i.size);
            data_obj.storage_index_ = db_type;
            COMMON_DBGVAR(data_obj);
            operation.Write(data_obj, data_obj);
        }
    }

    else if (lib_type == IOLib::NIOBE){
        auto client = symbios::Client();
        for (auto &i : objs) {
            auto data_obj = Data();
            data_obj.id_= i.id_;
            data_obj.position_=i.position_;
            data_obj.buffer_= data.substr(i.position_+i.chunk_index*max_obj_size, i.size);
            data_obj.storage_index_ = db_type;
            client.StoreRequest(data_obj);
        }
    }

    else if (lib_type == IOLib::SYMBIOS){
        auto client = symbios::Client();

        for (auto &i : objs) {
            auto data_obj = Data();
            data_obj.id_= i.id_;
            data_obj.position_=i.position_;
            data_obj.buffer_= data.substr(i.position_+i.chunk_index*max_obj_size, i.size);
            data_obj.storage_index_ = db_type;
            client.StoreRequest(data_obj);
        }
    }
    else{
        exit(3); // throw("INVALID LIB");
    }
}

void LibHandler::do_mapped_read() {
    DataDescriptor src = {file_,0,  data.size(), 0 };
    DataDescriptor read_src = {file_, 0,  data.size(), 0 };
    DataMapper mapper_(db_type, max_obj_size);
    auto objs = mapper_.map(src);
    auto read_objs = mapper_.map(read_src);

    if (lib_type == IOLib::IRIS){
        doOp operation(db_type);
        for (auto &i : read_objs){
            auto data_obj = Data();
            data_obj.id_= i.id_;
            data_obj.position_=i.position_;
            data_obj.buffer_.resize(i.size);
            data_obj.storage_index_ = db_type;
            operation.Read(data_obj, data_obj);
            COMMON_DBGVAR2(data_obj, i);
            std::cout<<i.position_<<':'<<data_obj.buffer_<<std::endl;
        }
    }

    else if (lib_type == IOLib::NIOBE){
        auto client = symbios::Client();
        for (auto &i : read_objs){
            auto data_obj = Data();
            data_obj.id_= i.id_;
            data_obj.position_=i.position_;
            data_obj.buffer_.resize(i.size);
            data_obj.storage_index_ = db_type;
            client.LocateRequest(data_obj);
            COMMON_DBGVAR2(data_obj, i);
            std::cout<<data_obj.buffer_<<std::endl;
        }

    }

    else if (lib_type == IOLib::SYMBIOS){
        auto client = symbios::Client();
        for (auto &i : read_objs){
            auto data_obj = Data();
            data_obj.id_= i.id_;
            data_obj.position_=i.position_;
            data_obj.buffer_.resize(i.size);
            data_obj.storage_index_ = db_type;
            client.LocateRequest(data_obj);
            COMMON_DBGVAR2(data_obj, i);
            std::cout<<data_obj.buffer_<<std::endl;
        }
    }
    else{
        exit(3); // throw("INVALID LIB");
    }
}

LibHandler::LibHandler(std::string file__, std::string data_, uint16_t lib_type_, uint16_t db_type_, uint16_t max_obj_size_) {
    lib_type = lib_type_;
    db_type = db_type_;
    max_obj_size = max_obj_size_;
    file_ = file__;
    data = data_;
}


//// DATA MAPPER

DataMapper::DataMapper(uint16_t type_, uint maxObjSize_) {
    storage_type_ = type_;
    maxObjSize = maxObjSize_;
}

std::vector<DataDescriptor> LibHandler::map_data() {
    DataMapper mapper_(db_type, max_obj_size);
    objs = mapper_.map(src);
    return objs;
}


std::vector<DataDescriptor> DataMapper::map(DataDescriptor &src){
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

///// END //////////