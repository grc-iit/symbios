#include <symbios/server/server.h>
#include <common/debug.h>
#include <symbios/data_distribution/data_distribution_factory.h>

symbios::Server::Server(){
    SYMBIOS_CONF->ConfigureSymbiosServer();
    auto basket=BASKET_CONF;
    rpc=basket::Singleton<RPCFactory>::GetInstance()->GetRPC(BASKET_CONF->RPC_PORT);
    std::function<int(Data&)> functionStoreRequest(std::bind(&Server::StoreRequest, this, std::placeholders::_1));
    std::function<Data(Data&)> functionLocateRequest(std::bind(&Server::LocateRequest, this, std::placeholders::_1));
    rpc->bind("StoreRequest", functionStoreRequest);
    rpc->bind("LocateRequest", functionLocateRequest);
}

void symbios::Server::RunInternal(std::future<void> futureObj) {
    while(futureObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout){
        usleep(10000);
    }
}

int symbios::Server::StoreRequest(Data &request){
    auto tracer=common::debug::AutoTrace(std::string("symbios::Server::StoreRequest"), request);
    auto dde = basket::Singleton<DataDistributionEngineFactory>::GetInstance()->GetDataDistributionEngine(SYMBIOS_CONF->DATA_DISTRIBUTION_POLICY);
    auto distributions = dde->Distribute(request);


    return rand();
}

Data symbios::Server::LocateRequest(Data &request){
    auto tracer=common::debug::AutoTrace(std::string("symbios::Server::LocateRequest"), request);
    auto dde = basket::Singleton<DataDistributionEngineFactory>::GetInstance()->GetDataDistributionEngine(SYMBIOS_CONF->DATA_DISTRIBUTION_POLICY);
    auto distributions = dde->Distribute(request);


    return request;
}

void symbios::Server::Run(std::future<void> futureObj) {
    RunInternal(std::move(futureObj));
}
