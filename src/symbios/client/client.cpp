#include <common/debug.h>
#include <rpc/client.h>
#include <symbios/client/client.h>
#include <unistd.h>

/*
 * Default COnstructor
 * 1) Initialization and get rpc instance
 */
//: fileDescriptorMap("FileDescriptors", BASKET_CONF->RPC_PORT)
symbios::Client::Client(){
    SYMBIOS_CONF->ConfigureSymbiosClient();
    auto basket=BASKET_CONF;
    rpc=basket::Singleton<RPCFactory>::GetInstance()->GetRPC(BASKET_CONF->RPC_PORT); //TODO: use another port?
    COMMON_DBGVAR(BASKET_CONF->RPC_PORT);
}

/*
 * Store Request interface
 * 1) Sending store request to server through rpc
 * @Parameter source: the source data information which include the data you want to store
 * @Parameter detination: the destination information which indicates the io storage information
 */
void symbios::Client::StoreRequest(Data &source,Data &destination) {
    AUTO_TRACER("symbios::Client::StoreRequest", request);
    int server = rand() % BASKET_CONF->NUM_SERVERS;
    auto num_servers = rpc->call<RPCLIB_MSGPACK::object_handle>(server, "StoreRequest", source,destination) .as<int>();
    COMMON_DBGVAR(num_servers);
}

/*
 * Locate Request interface
 * 1) Sending locate request to server through rpc
 * @Parameter source: the source data information which indicates the io storage information
 * @Parameter destination: the destination information which will include the data attained from the server
 */
void symbios::Client::LocateRequest(Data &source,Data &destination) {

    AUTO_TRACER("symbios::Client::LocateRequest", request);
    int server = rand() % BASKET_CONF->NUM_SERVERS;
    auto ret = rpc->call<RPCLIB_MSGPACK::object_handle>(server, "LocateRequest", source,destination).as<Data>();
    if(destination.buffer_==NULL) destination.buffer_ = static_cast<char *>(malloc(ret.data_size_));
    memcpy(destination.buffer_,ret.buffer_,ret.data_size_);
    COMMON_DBGVAR(request.buffer_);

}

/*
 * Delete Request interface
 * 1) Sending delete request to server through rpc
 * @Parameter request: the data information that you want to delete from the server
 * @return bool
 */
bool symbios::Client::Delete(Data &request) {
    AUTO_TRACER("symbios::Client::LocateRequest", request);
    int server = rand() % BASKET_CONF->NUM_SERVERS;
    return rpc->call<RPCLIB_MSGPACK::object_handle>(server, "DeleteRequest", request).as<bool>();
}

/*
 * Get data size interface
 * 1) Sending Size request to server through rpc
 * @Parameter request: the data information that describes the filename or key
 * @return size_t
 */
size_t symbios::Client::Size(Data &request) {
    AUTO_TRACER("symbios::Client::LocateRequest", request);
    int server = rand() % BASKET_CONF->NUM_SERVERS;
    return rpc->call<RPCLIB_MSGPACK::object_handle>(server, "SizeRequest", request).as<size_t>();
}


