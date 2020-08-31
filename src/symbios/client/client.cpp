#include <symbios/client/client.h>
#include <rpc/client.h>
//: fileDescriptorMap("FileDescriptors", BASKET_CONF->RPC_PORT)
symbios::Client::Client(){
    SYMBIOS_CONF->ConfigureSymbiosClient();
    auto basket=BASKET_CONF;
    rpc=basket::Singleton<RPCFactory>::GetInstance()->GetRPC(BASKET_CONF->RPC_PORT); //TODO: use another port?
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

//bool symbios::Client::addOrUpdateFileDescriptorPool(FILE* fh, CharStruct filename) {
//    auto map_locator = fileDescriptorMap.Get(fh);
//    if (map_locator.first)
//        fileDescriptorMap.Erase(fh);
//    return fileDescriptorMap.Put(fh, filename);
//}
//
//bool symbios::Client::isFileDescriptorTracked(FILE* fh) {
//    auto map_locator=fileDescriptorMap.Get(fh);
//    return map_locator.first;
//}
//
//bool symbios::Client::deleteFileDescriptorFromPool(FILE* fh) {
//    auto map_locator = fileDescriptorMap.Get(fh);
//    if (map_locator.first)
//        return fileDescriptorMap.Erase(fh).first;
//    else
//        return map_locator.first;
//}
//
//std::pair<bool, CharStruct> symbios::Client::getFileNameFromMap(FILE* fh) {
//    return fileDescriptorMap.Get(fh);
//}
