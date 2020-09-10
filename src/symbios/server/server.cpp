#include <symbios/server/server.h>
#include <common/debug.h>
#include <symbios/data_distribution/data_distribution_factory.h>
#include <symbios/metadata_orchestrator/metadata_orchestrator.h>
#include <symbios/io_clients/io_factory.h>

/*
 * Default Constructor
 * 1) Initialize and get rpc instance
 * 2) preload DataDistributionEngineFactory/MetadataOrchestrator/IOFactory classes
 */
symbios::Server::Server(){
    SYMBIOS_CONF->ConfigureSymbiosServer();
    auto basket=BASKET_CONF;
    rpc=basket::Singleton<RPCFactory>::GetInstance()->GetRPC(BASKET_CONF->RPC_PORT);
    std::function<int(Data&,Data&)> functionStoreRequest(std::bind(&Server::Store, this, std::placeholders::_1, std::placeholders::_2));
    std::function<Data(Data&,Data&)> functionLocateRequest(std::bind(&Server::Locate, this, std::placeholders::_1, std::placeholders::_2));
    std::function<size_t(Data&)> functionSizeRequest(std::bind(&Server::Size, this, std::placeholders::_1));
    std::function<bool(Data&)> functionDeleteRequest(std::bind(&Server::Delete, this, std::placeholders::_1));
    rpc->bind("StoreRequest", functionStoreRequest);
    rpc->bind("LocateRequest", functionLocateRequest);
    rpc->bind("DeleteRequest", functionDeleteRequest);
    rpc->bind("SizeRequest", functionSizeRequest);
    /**
     * Preload classes.
     */
    basket::Singleton<DataDistributionEngineFactory>::GetInstance()->GetDataDistributionEngine(SYMBIOS_CONF->DATA_DISTRIBUTION_POLICY);
    basket::Singleton<MetadataOrchestrator>::GetInstance();
    basket::Singleton<IOFactory>::GetInstance();

}

/*
 * Running interface which is called by external module
 */
void symbios::Server::Run(std::future<void> futureObj) {
    RunInternal(std::move(futureObj));
}

/*
 * Internal running interface
 */
void symbios::Server::RunInternal(std::future<void> futureObj) {
    while(futureObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout){
        usleep(10000);
    }
}

/*
 * Store the source request data
 * 1) Getting the data distributions according to the data distribution policy
 * 2) Calling MetadataOrchestrator module to store the metadata information
 * 3) Iterating all the data distributions and storing data into the responding storage.
 * @Parameter source: the source data information that contains the data you want to store
 * @Parameter destination: the original destination information that includes the io related information
 */
int symbios::Server::Store(Data &source, Data &destination){
    AUTO_TRACER(std::string("symbios::Server::StoreRequest"), request);
    auto dde = basket::Singleton<DataDistributionEngineFactory>::GetInstance()->GetDataDistributionEngine(SYMBIOS_CONF->DATA_DISTRIBUTION_POLICY);
    auto distributions = dde->Distribute(source,destination);
    basket::Singleton<MetadataOrchestrator>::GetInstance()->Store(destination,distributions);
    for(auto distribution:distributions){
        basket::Singleton<IOFactory>::GetInstance()->GetIOClient(distribution.destination_data_.storage_index_)->Write(distribution.source_data_, distribution.destination_data_);
        COMMON_DBGMSG("Storing data in "<<distribution.destination_data_.storage_index_);
    }

    return SYMBIOS_CONF->SERVER_COUNT;
}

/*
 * Locate the source request data
 * 1) Getting the data distributions from MetadataOrchestrator by locating the request data information
 * 2) Iterating all the data distributions and getting data from the responding storage.
 * 3) Merging all the data getting from storages
 * @Parameter source: the source data information that you want to locate
 * @Parameter destination: the destination information which will contains the data attained from the responding data distributions
 * @return Data: the getted data information
 */
Data symbios::Server::Locate(Data &source, Data &destination){
    AUTO_TRACER(std::string("symbios::Server::LocateRequest"), request);
    Metadata primary_metadata;
    auto distributions = basket::Singleton<MetadataOrchestrator>::GetInstance()->Locate(source, primary_metadata);
    int total_size= 0;
    for(auto &distribution:distributions){
        basket::Singleton<IOFactory>::GetInstance()->GetIOClient(distribution.destination_data_.storage_index_)->Read(distribution.source_data_,distribution.destination_data_);
        total_size+=distribution.destination_data_.data_size_;
    }
    destination.buffer_= static_cast<char *>(malloc(total_size));
    long start=0;
    for(auto &distribution:distributions){
        memcpy(destination.buffer_+start,distribution.destination_data_.buffer_+ distribution.destination_data_.position_, distribution.destination_data_.data_size_- distribution.destination_data_.position_);
        free(distribution.destination_data_.buffer_);
        start+=distribution.destination_data_.data_size_ - distribution.destination_data_.position_;
    }
    COMMON_DBGVAR(destination.buffer_);
    return destination;
}

/*
 * Get data size
 * 1) Getting the data distributions from MetadataOrchestrator by locating the request data information
 * 2) Iterating all the data distributions and getting data size from the responding storage
 * 3) Calculating the total data size
 * @Parameter request: the data request information
 * @return size_t: the data size
 */
size_t symbios::Server::Size(Data &request) {
    AUTO_TRACER(std::string("symbios::Server::Size"), request);
    Metadata primary_metadata;
    std::vector<DataDistribution> distributions;
    try {
        distributions = basket::Singleton<MetadataOrchestrator>::GetInstance()->Locate(request, primary_metadata);
    } catch (ErrorException e) {
        return 0;
    }
    int total_size= 0;
    for(auto &distribution:distributions){
        total_size+=basket::Singleton<IOFactory>::GetInstance()->GetIOClient(distribution.destination_data_.storage_index_)->Size(distribution.destination_data_);
    }
    return total_size;
}

/*
 * Delete the request data
 * 1) Getting the data distributions from MetadataOrchestrator by locating the request data information
 * 2) Iterating all the data distributions and removing data from the responding storage.
 * 3) Calling MetadataOrchestrator module to delete all the metadata information
 * @Parameter request: the data information that you want to locate
 * @return bool
 */
bool symbios::Server::Delete(Data &request) {
    AUTO_TRACER(std::string("symbios::Server::Delete"), request);
    Metadata primary_metadata;
    std::vector<DataDistribution> distributions;
    try {
        distributions = basket::Singleton<MetadataOrchestrator>::GetInstance()->Locate(request, primary_metadata);
    } catch (ErrorException e) {
        return false;
    }
    bool status = true;
    for(auto &distribution:distributions){
        status = status && basket::Singleton<IOFactory>::GetInstance()->GetIOClient(distribution.destination_data_.storage_index_)->Remove(distribution.source_data_);
    }
    return status && basket::Singleton<MetadataOrchestrator>::GetInstance()->Delete(request);
}




