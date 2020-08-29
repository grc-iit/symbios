#include <symbios/client/client.h>
#include <rpc/client.h>

symbios::Client::Client(){
    SYMBIOS_CONF->ConfigureSymbiosClient();
    auto basket=BASKET_CONF;
    rpc=basket::Singleton<RPCFactory>::GetInstance()->GetRPC(BASKET_CONF->RPC_PORT);
}

void symbios::Client::send_request(Data data) {
    int server = rand()% BASKET_CONF->NUM_SERVERS;
    auto num_servers = rpc->call<RPCLIB_MSGPACK::object_handle>(server, "Posix_Request", data).as<int>();
    printf("response from server %d\n",num_servers);
}