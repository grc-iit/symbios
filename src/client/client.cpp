//
// Created by mani on 8/24/2020.
//

#include "../../include/symbios/client.h"
#include <basket.h>
#include <iostream>
#include <mpi.h>
#include "../server/configuration_manager.h"
#include "../server/data_structures.h"

void send_request(Request req) {
    int my_server = 0;
    basket::queue<Request> int_queue("SERVER_REQUEST_QUEUE",SYMBIOS_CONF->CLIENT_PORT);

    int_queue.Push(req, my_server);
}
