//
// Created by mani on 8/24/2020.
//

#ifndef SYMBIOS_SERVER_H
#define SYMBIOS_SERVER_H

#include <basket.h>
#include "data_structures.h"

namespace symbios{
    class Server {
    private:
        basket::queue<Request>  server_request_queue;
        /**
         * Server hosting
         */
        void RunInternal(std::future<void> futureObj);
    public:
        Server(std::future<void> futureObj);
    };
}


#endif //SYMBIOS_SERVER_H
