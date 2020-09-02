//
// Created by mani on 8/24/2020.
//

#ifndef SYMBIOS_SERVER_H
#define SYMBIOS_SERVER_H

#include <basket.h>
#include <symbios/common/data_structure.h>
#include <symbios/common/configuration_manager.h>

namespace symbios {
    class Server {
    private:
        std::shared_ptr<RPC> rpc;

        void RunInternal(std::future<void> futureObj);
    public:
        void Run(std::future<void> futureObj);
        explicit Server();
        int Store(Data &request);
        Data Locate(Data &request);
        size_t Size(Data &request);
        bool Delete(Data &request);
    };
}

#endif //SYMBIOS_SERVER_H