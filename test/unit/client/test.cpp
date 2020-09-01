//
// Created by mani on 8/24/2020.
//

#include <symbios/client/client.h>
#include <common/debug.h>

int main(int argc, char * argv[]){
    auto tracer=common::debug::AutoTrace(std::string("client::main"));
    MPI_Init(&argc,&argv);
    MPI_Barrier(MPI_COMM_WORLD);
    if(argc > 1) SYMBIOS_CONF->CONFIGURATION_FILE=argv[1];
    BASKET_CONF->BACKED_FILE_DIR=SYMBIOS_CONF->SERVER_DIR;
    auto client = symbios::Client();
    auto data = Data();
    data.id_="filename113213";
    data.position_=0;
    data.buffer_= "Hello";
    data.storage_index_=0;
    client.StoreRequest(data);
    data.buffer_= std::string().data();
    data.storage_index_=0;
    client.LocateRequest(data);

    COMMON_DBGVAR(data.buffer_.data());
    MPI_Finalize();
    return 0;
}