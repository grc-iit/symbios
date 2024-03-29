#include <common/debug.h>
#include <rpc/client.h>
#include <symbios/client/client.h>
#include <unistd.h>

//: fileDescriptorMap("FileDescriptors", BASKET_CONF->RPC_PORT)
symbios::Client::Client(){
    SYMBIOS_CONF->ConfigureSymbiosClient();
    auto basket=BASKET_CONF;
    rpc=basket::Singleton<RPCFactory>::GetInstance()->GetRPC(BASKET_CONF->RPC_PORT); //TODO: use another port?
    COMMON_DBGVAR(BASKET_CONF->RPC_PORT);
}

void symbios::Client::StoreRequest(Data &source,Data &destination) {
    AUTO_TRACER("symbios::Client::StoreRequest", request);
    int server = rand() % BASKET_CONF->NUM_SERVERS;
    auto num_servers = rpc->call<RPCLIB_MSGPACK::object_handle>(server, "StoreRequest", source,destination) .as<int>();
    COMMON_DBGVAR(num_servers);
}

void symbios::Client::LocateRequest(Data &source,Data &destination) {

    AUTO_TRACER("symbios::Client::LocateRequest", request);
    int server = rand() % BASKET_CONF->NUM_SERVERS;
    auto ret = rpc->call<RPCLIB_MSGPACK::object_handle>(server, "LocateRequest", source,destination).as<Data>();
    if(destination.buffer_==NULL) destination.buffer_ = static_cast<char *>(malloc(ret.data_size_));
    memcpy(destination.buffer_,ret.buffer_,ret.data_size_);
    COMMON_DBGVAR(request.buffer_);

}

bool symbios::Client::Delete(Data &request) {
    AUTO_TRACER("symbios::Client::LocateRequest", request);
    int server = rand() % BASKET_CONF->NUM_SERVERS;
    return rpc->call<RPCLIB_MSGPACK::object_handle>(server, "DeleteRequest", request).as<bool>();
}

size_t symbios::Client::Size(Data &request) {
    AUTO_TRACER("symbios::Client::LocateRequest", request);
    int server = rand() % BASKET_CONF->NUM_SERVERS;
    return rpc->call<RPCLIB_MSGPACK::object_handle>(server, "SizeRequest", request).as<size_t>();
}


