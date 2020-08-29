//
// Created by mani on 8/24/2020.
//

#include <mpi.h>
#include <basket.h>
#include <symbios/server/server.h>
#include <symbios/server/Daemon.h>
#include <symbios/common/configuration_manager.h>

int main(int argc, char* argv[]){
    MPI_Init_thread(&argc,&argv,true,NULL);
    MPI_Barrier(MPI_COMM_WORLD);
    if(argc > 1) SYMBIOS_CONF->CONFIGURATION_FILE=argv[1];
    BASKET_CONF->BACKED_FILE_DIR=SYMBIOS_CONF->SERVER_DIR;

    auto daemon = symbios::Daemon<symbios::Server>();

    while(true) sleep(1);
}