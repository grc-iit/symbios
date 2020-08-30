#include <symbios/client/client.h>
#include <rpc/client.h>

symbios::Client::Client(){
    SYMBIOS_CONF->ConfigureSymbiosClient();
    auto basket=BASKET_CONF;
    rpc=basket::Singleton<RPCFactory>::GetInstance()->GetRPC(BASKET_CONF->RPC_PORT);
}

void symbios::Client::StoreRequest(Data &request) {
    int server = rand()% BASKET_CONF->NUM_SERVERS;
    auto num_servers = rpc->call<RPCLIB_MSGPACK::object_handle>(server, "StoreRequest", request).as<int>();
    printf("response from server %d\n",num_servers);
}

void symbios::Client::LocateRequest(Data &request) {
    int server = rand()% BASKET_CONF->NUM_SERVERS;
    auto ret = rpc->call<RPCLIB_MSGPACK::object_handle>(server, "LocateRequest", request).as<Data>();
    request.buffer_=malloc(ret.data_size_);
    memcpy(request.buffer_,ret.buffer_,ret.data_size_);
    request.data_size_ = ret.data_size_;
}
