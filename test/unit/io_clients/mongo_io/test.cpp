#include <cstdint>
#include <iostream>
#include <vector>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/instance.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <mongocxx/exception/query_exception.hpp>
#include <mongocxx/exception/bulk_write_exception.hpp>
#include <symbios/io_clients/io_factory.h>

int main(int argc, char*argv[]){
    MPI_Init(&argc,&argv);
    std::string key("test");
    std::string value("value");
    try{
        Data destination;
        Data source;
        destination.id_=key;
        source.buffer_=(void*)value.c_str();
        source.data_size_=strlen(value.c_str())+1;
        basket::Singleton<IOFactory>::GetInstance()->GetIOClient(2)->Write(source,destination);
    } catch (mongocxx::bulk_write_exception ex){
        //throw ErrorException(MONGODB_SERVER_SIDE_FAILED);
        std::cout << "write error....." << std::endl;
    }

    std::cout << "read the written data...." << std::endl;
    try {
        Data source;
        source.id_=key;
        basket::Singleton<IOFactory>::GetInstance()->GetIOClient(2)->Read(source,source);
    } catch (mongocxx::query_exception ex){
        //throw ErrorException(MONGODB_SERVER_SIDE_FAILED);
    }
    MPI_Finalize();
}