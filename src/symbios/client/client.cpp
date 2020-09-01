#include <common/debug.h>
#include <rpc/client.h>
#include <symbios/client/client.h>
#include <unistd.h>

symbios::Client::Client() {
    auto tracer = common::debug::AutoTrace("symbios::Client::Client");
    SYMBIOS_CONF->ConfigureSymbiosClient();
    rpc = basket::Singleton<RPCFactory>::GetInstance()->GetRPC(BASKET_CONF->RPC_PORT);
    COMMON_DBGVAR(BASKET_CONF->RPC_PORT);
}

void symbios::Client::StoreRequest(Data &request) {
    auto tracer = common::debug::AutoTrace("symbios::Client::StoreRequest", request);
    int server = rand() % BASKET_CONF->NUM_SERVERS;
    auto num_servers = rpc->call<RPCLIB_MSGPACK::object_handle>(server, "StoreRequest", request) .as<int>();
    COMMON_DBGVAR(num_servers);
}

void symbios::Client::LocateRequest(Data &request) {

    auto tracer = common::debug::AutoTrace("symbios::Client::LocateRequest", request);
    int server = rand() % BASKET_CONF->NUM_SERVERS;
    auto ret = rpc->call<RPCLIB_MSGPACK::object_handle>(server, "LocateRequest", request).as<Data>();
    request.buffer_ = ret.buffer_;
    COMMON_DBGVAR(request.buffer_);

}
