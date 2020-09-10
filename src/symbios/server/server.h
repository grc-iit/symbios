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
        /*Internal running interface*/
        void RunInternal(std::future<void> futureObj);
    public:
        /*Running interface called by external module*/
        void Run(std::future<void> futureObj);
        /*Default Constructor which need to be called explicitly*/
        explicit Server();
        /*Store request interface*/
        int Store(Data &source, Data &destination);
        /*Locate request interface*/
        Data Locate(Data &source, Data &destination);
        /*Get data size interface*/
        size_t Size(Data &request);
        /*Delete request interface*/
        bool Delete(Data &request);
    };
}

#endif //SYMBIOS_SERVER_H