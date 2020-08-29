//
// Created by mani on 8/24/2020.
//

#include <mpi.h>
#include <basket.h>
#include <symbios/server/daemon.h>
#include <symbios/server/server.h>
#include <symbios/common/configuration_manager.h>

int main(int argc, char* argv[]){
    MPI_Init_thread(&argc,&argv,true,NULL);
    MPI_Barrier(MPI_COMM_WORLD);
    if(argc > 1) SYMBIOS_CONF->CONFIGURATION_FILE=argv[1];
    BASKET_CONF->BACKED_FILE_DIR=SYMBIOS_CONF->SERVER_DIR;
    CharStruct log = "symbios_server.log";
    auto daemon = basket::Singleton<symbios::Daemon<symbios::Server>>::GetInstance(log);
    while(true) sleep(1);
}