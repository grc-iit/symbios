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
        explicit Server(std::future<void> futureObj);
        int PosixRequest(Data req);
    };
}

#endif //SYMBIOS_SERVER_H