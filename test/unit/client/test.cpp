//
// Created by mani on 8/24/2020.
//

#include <symbios/client/client.h>

int main(int argc, char * argv[]){
    MPI_Init(&argc,&argv);
    FILE *outfile = std::fopen(argv[1], "r");
    if (outfile == NULL) {
        printf("Symbios configuration not found %s %d\n",strerror( errno ),errno);
        exit(EXIT_FAILURE);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if(argc > 1) SYMBIOS_CONF->CONFIGURATION_FILE=argv[1];
    BASKET_CONF->BACKED_FILE_DIR=SYMBIOS_CONF->SERVER_DIR;
    auto client = symbios::Client();
    auto data = Data();
    data.id_="filename";
    data.position_=0;
    data.buffer_= (void *) "Hello";
    data.data_size_=strlen("Hello")+1;
    client.send_request(data);
    MPI_Finalize();
    return 0;
}