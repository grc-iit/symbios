//
// Created by neeraj on 9/1/20.
//

#include <common/iris.h>

void slice_str(const char * str, char * buffer, size_t start, size_t end){
    size_t j = 0;
    for ( size_t i = start; i <= end; ++i ) {
        buffer[j++] = str[i];
    }
    buffer[j] = 0;
}

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

void doOp::Remove(Data &source) {
    client->Remove(source);
}

doOp::~doOp() {}


//// LibHandler

void LibHandler::run(OPType op_type, long offset, size_t request_size, char* data) {
    //map_data() ?
    if (op_type == OPType::READ) do_mapped_read(offset, request_size, data);
    else if (op_type == OPType::WRITE) do_mapped_write(offset, request_size, data);
    else if (op_type == OPType::DELETE) do_mapped_delete(offset, request_size, data);

}

void LibHandler::do_mapped_write(long offset, size_t request_size, char *data) {



    if (lib_type == IOLib::IRIS){
        DataDescriptor src = {file_, offset,  request_size, 0 };
        DataMapper mapper_(db_type, max_obj_size);
        auto objs = mapper_.map(src);

        uint min_chunk_index = objs[0].chunk_index;
        for (auto &i : objs) {
            if (i.chunk_index < min_chunk_index) {
                min_chunk_index = i.chunk_index;
            }
        }
        doOp operation(db_type);
        long source_position = 0;
        for (auto &i : objs){
            auto dest_data = Data();
            auto src_data = Data();
            dest_data.id_= i.id_;
            dest_data.position_=i.position_;
            dest_data.data_size_=i.size;
            src_data.buffer_ = data;
            src_data.position_ = source_position;
            src_data.data_size_ = i.size;
            source_position += src_data.data_size_;
            dest_data.storage_index_ = db_type;
            // COMMON_DBGVAR(data_obj);
            operation.Write(src_data, dest_data);
        }
    }

    else if (lib_type == IOLib::SYMBIOS || lib_type == IOLib::NIOBE){
        auto destination = Data();
        destination.id_= file_;
        destination.position_= offset;
        destination.data_size_= request_size;
        destination.storage_index_ = db_type;
        auto source = destination;
        source.buffer_ = data;
        source.position_=0;
        symbios_client->StoreRequest(source, destination);
    }
    else{
        exit(3); // throw("INVALID LIB");
    }
}

void LibHandler::do_mapped_delete(long offset, size_t request_size, char *data){
    DataDescriptor src = {file_, offset,  request_size, 0 };
    DataMapper mapper_(db_type, max_obj_size);
    auto objs = mapper_.map(src);

    uint min_chunk_index = objs[0].chunk_index;
    for (auto &i : objs) {
        if (i.chunk_index < min_chunk_index) {
            min_chunk_index = i.chunk_index;
        }
    }

    if (lib_type == IOLib::IRIS){
        doOp operation(db_type);
        for (auto &i : objs){
            auto dest_data = Data();
            auto src_data = Data();
            dest_data.id_= i.id_;
            dest_data.position_=i.position_;
            src_data.buffer_ = data + (i.chunk_index-min_chunk_index)*max_obj_size;
            src_data.position_ = 0;
            src_data.data_size_ = i.size;

            dest_data.storage_index_ = db_type;
            // COMMON_DBGVAR(data_obj);
            operation.Remove(dest_data);
        }
    }

    else if (lib_type == IOLib::SYMBIOS || lib_type == IOLib::NIOBE){
        auto source = Data();
        source.buffer_ = data;
        source.id_= file_;
        source.position_= offset;
        source.data_size_= request_size;
        source.storage_index_ = db_type;
        symbios_client->Delete(source);
    }
    else{
        exit(3); // throw("INVALID LIB");
    }
}

void LibHandler::do_mapped_read(long offset, size_t request_size, char *data) {
    DataDescriptor read_src = {file_, offset,  request_size, 0 };
    DataMapper mapper_(db_type, max_obj_size);
    auto read_objs = mapper_.map(read_src);
    int data_offset = 0;

    if (lib_type == IOLib::IRIS){
        doOp operation(db_type);
        for (auto &i : read_objs){
            auto src_data = Data();
            auto dest_data = Data();
            src_data.id_= i.id_;
            src_data.position_=i.position_;
            dest_data.position_=0;
            if (data_offset + i.size > request_size) {
                src_data.data_size_= i.size;
                dest_data.data_size_=request_size - data_offset;
            }
            else {
                src_data.data_size_= i.size;
                dest_data.data_size_=i.size;
            }
            
            src_data.storage_index_ = db_type;

            operation.Read(src_data, dest_data);
            if (print_p) {
                dest_data.buffer_[i.size] = '\0';
                std::cout << dest_data.buffer_;
            }
            free(dest_data.buffer_);
            COMMON_DBGVAR2(src_data, i);
            data_offset += src_data.data_size_;
            if (data_offset > request_size) {
                break;
            }
        }
        if (print_p) {
            std::cout << std::endl;
        }
    }

    else if (lib_type == IOLib::SYMBIOS || lib_type == IOLib::NIOBE){
        auto source = Data();
        source.buffer_ = data;
        source.id_= file_;
        source.position_= offset;
        source.data_size_= request_size;
        source.storage_index_ = db_type;
        auto destination = source;
        destination.buffer_ = data;
        destination.position_=0;
        symbios_client->LocateRequest(source,destination);
    }
    else{
        exit(3); // throw("INVALID LIB");
    }
}

LibHandler::LibHandler(std::string file__, IOLib lib_type_, uint16_t io_type_, long max_obj_size_, bool print_p_, std::string symbios_conf_) {
    lib_type = lib_type_;
    db_type = io_type_;
    max_obj_size = max_obj_size_;
    file_ = file__;
    print_p = print_p_;
    symbios_conf = symbios_conf_;

    if (lib_type == IOLib::SYMBIOS || lib_type == IOLib::NIOBE) {
        SYMBIOS_CONF->CONFIGURATION_FILE=symbios_conf;
        BASKET_CONF->BACKED_FILE_DIR=SYMBIOS_CONF->SERVER_DIR;
        symbios_client = new symbios::Client();
        MPI_Barrier(MPI_COMM_WORLD);
        auto solution = SYMBIOS_CONF->STORAGE_SOLUTIONS[db_type];
        auto newSSMap = std::unordered_map<uint16_t, std::shared_ptr<StorageSolution>>();
        auto new_SS = std::pair<uint16_t, std::shared_ptr<StorageSolution>>(0, solution);
        newSSMap.insert(new_SS);
        SYMBIOS_CONF->STORAGE_SOLUTIONS = newSSMap;
        SYMBIOS_CONF->DATA_DISTRIBUTION_POLICY=RANDOM_POLICY;
        MPI_Barrier(MPI_COMM_WORLD);

    }
}

LibHandler::~LibHandler() {
    if (lib_type == IOLib::SYMBIOS || lib_type == IOLib::NIOBE) {
        delete symbios_client;
    }
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
    long curr_pos = src.position_;
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

    // std::vector<DataDescriptor> objects;

    // // std::cout << "size" << src.size << std::endl;
    // std::size_t remainingOperationSize = src.size;
    // int curr_pos = src.position_;
    // std::cout << "objs construction: " << src.position_ << " " << src.size << std::endl;
    // while(remainingOperationSize!=0) {
    //     DataDescriptor obj;
    //     auto index = curr_pos / maxObjSize;
    //     obj.id_=src.id_+ std::to_string(index);
    //     obj.position_ = curr_pos ;
    //     // (maxObjSize - (obj.position_ % maxObjSize))
    //     obj.size = (remainingOperationSize > maxObjSize) ? (maxObjSize - (obj.position_ % maxObjSize)) : remainingOperationSize;
    //     remainingOperationSize -= obj.size;
    //     curr_pos += obj.size;
    //     obj.chunk_index = index;
    //     objects.push_back(obj);
    // }
    // std::cout << objects.size() << std::endl;

    // return objects;
}

///// END //////////
