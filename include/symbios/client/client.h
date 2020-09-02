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
        Client();

        void StoreRequest(Data &request);

        void LocateRequest(Data &request);

        bool Delete(Data &request);

        size_t Size(Data &request);
    };
}


#endif //SYMBIOS_CLIENT_H