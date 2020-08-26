//
// Created by jaime on 8/26/2020.
//

#include "server.h"
#include "configuration_manager.h"
symbios::Server::Server(std::future<void> futureObj):server_request_queue("SERVER_REQUEST_QUEUE",SYMBIOS_CONF->CHRONOPLAYER_SERVER_PORT) {
    RunInternal(std::move(futureObj));
}

void symbios::Server::RunInternal(std::future<void> futureObj) {
    while(futureObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout){
        auto num_elements =  server_request_queue.LocalSize();
        auto events = std::vector<Request>();
        int count=0;
        while(count < num_elements && server_request_queue.LocalSize() != 0){
            auto ele = server_request_queue.LocalPop();
            if(ele.first) events.push_back(ele.second);
        }
        // do Something with request
        usleep(10000);
    }
}
