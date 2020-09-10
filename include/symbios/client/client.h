#ifndef SYMBIOS_CLIENT_H
#define SYMBIOS_CLIENT_H

#include <basket.h>
#include <iostream>
#include <mpi.h>
#include <symbios/common/configuration_manager.h>
#include <symbios/common/data_structure.h>
#include "dlfcn.h"

namespace symbios {
    class Client {
    private:
        std::shared_ptr<RPC> rpc;
    public:
        /*Default Constructor*/
        Client();
        /*Store source request interface*/
        void StoreRequest(Data &source,Data &destination);
        /*Locate source request interface*/
        void LocateRequest(Data &source,Data &destination);
        /*Delete request interface*/
        bool Delete(Data &request);
        /*Get data size interface*/
        size_t Size(Data &request);
    };
}


#endif //SYMBIOS_CLIENT_H