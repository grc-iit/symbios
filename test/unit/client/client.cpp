//
// Created by mani on 8/24/2020.
//

#include <symbios/client/client.h>

int main(int argc, char * argv[]){
    MPI_Init(&argc,&argv);
    MPI_Barrier(MPI_COMM_WORLD);
    if(argc > 1) SYMBIOS_CONF->CONFIGURATION_FILE=argv[1];
    BASKET_CONF->BACKED_FILE_DIR=SYMBIOS_CONF->SERVER_DIR;
    auto client = symbios::Client();
    auto data = Data();
    data.id_="filename";
    data.position_=0;
    data.buffer_= (void *) "Hello";
    data.data_size_=strlen("Hello")+1;
    data.storage_index_=0;
    client.StoreRequest(data);
    data.buffer_= std::string().data();
    client.LocateRequest(data);
    printf("Data recieved %s\n",(char*)data.buffer_);
    MPI_Finalize();
    return 0;
}