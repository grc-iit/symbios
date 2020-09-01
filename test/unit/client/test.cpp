//
// Created by mani on 8/24/2020.
//

#include <symbios/client/test.h>

int main(int argc, char * argv[]){
    MPI_Init(&argc,&argv);
    MPI_Barrier(MPI_COMM_WORLD);

//    int ioMode, distributionMode, requestSize, requestNumber;

    if(argc > 1){
        SYMBIOS_CONF->CONFIGURATION_FILE=argv[1];
//        IOMode = std::stoi(argv[2]);
//        DistributionMode
    }
    BASKET_CONF->BACKED_FILE_DIR=SYMBIOS_CONF->SERVER_DIR;
    auto client = symbios::Client();
    auto data = Data();
    data.id_="filename2";
    data.position_=0;
    data.buffer_= "Hello";
    data.storage_index_=0;
    client.StoreRequest(data);
    data.buffer_= std::string().data();
    data.storage_index_=2;
    client.LocateRequest(data);
    printf("Data recieved %s\n",data.buffer_.data());
    MPI_Finalize();
    return 0;
}