#include <symbios/server/server.h>
#include <common/debug.h>
#include <symbios/data_distribution/data_distribution_factory.h>
#include <symbios/metadata_orchestrator/metadata_orchestrator.h>
#include <symbios/io_clients/io_factory.h>

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
    basket::Singleton<MetadataOrchestrator>::GetInstance()->Store(request,distributions);
    for(auto distribution:distributions){
        basket::Singleton<IOFactory>::GetInstance()->GetIOClient(distribution.destination_data_.io_client_type_)->Write(distribution.source_data_,distribution.destination_data_);
        COMMON_DBGMSG("Storing data in "<<distribution.destination_data_.io_client_type_);
    }
    return SYMBIOS_CONF->SERVER_COUNT;
}

Data symbios::Server::LocateRequest(Data &request){
    auto tracer=common::debug::AutoTrace(std::string("symbios::Server::LocateRequest"), request);
    auto distributions = basket::Singleton<MetadataOrchestrator>::GetInstance()->Locate(request);
    for(auto distribution:distributions){
        basket::Singleton<IOFactory>::GetInstance()->GetIOClient(distribution.destination_data_.io_client_type_)->Read(distribution.source_data_,distribution.destination_data_);
    }
    return request;
}

void symbios::Server::Run(std::future<void> futureObj) {
    RunInternal(std::move(futureObj));
}

