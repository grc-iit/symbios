//
// Created by mani on 8/24/2020.
//

#include <symbios/metadata_orchestrator/metadata_orchestrator.h>
#include <symbios/common/configuration_manager.h>
#include <symbios/io_clients/io_factory.h>
#include <rpc/msgpack.hpp>
#include <symbios/common/error_definition.h>
#include <common/debug.h>

void MetadataOrchestrator::Store(Data &original_request, std::vector<Distribution> &distributions) {
    auto original_metadata = original_request;
    /**
     * Update primary index
     */
    original_metadata.position_ = 0;
    auto primary_metadata = Metadata();
    bool exists;
    auto existing_distributions = std::vector<Distribution>();
    try {
        existing_distributions = Locate(original_request,primary_metadata);
        exists=true;
    } catch (ErrorException e) {
        exists=false;
    }
    if(exists){
        for (auto distribution:distributions) {
            auto iter = primary_metadata.links_.find(distribution.destination_data_.position_);
            if(iter == primary_metadata.links_.end()){
                distribution.destination_data_.buffer_ = std::string().data();
                primary_metadata.links_.insert({distribution.destination_data_.position_, distribution.destination_data_});
            }else{
                distribution.destination_data_.storage_index_ = iter->second.storage_index_;
            }
        }
    }else{
        primary_metadata.is_link_=false;
        primary_metadata.storage_index_=original_request.storage_index_;
        for (auto distribution:distributions) {
            distribution.destination_data_.buffer_ = std::string().data();
            primary_metadata.links_.insert({distribution.destination_data_.position_, distribution.destination_data_});
        }
    }
    std::stringstream buffer;
    clmdep_msgpack::pack(buffer, primary_metadata);
    buffer.seekg(0);
    std::string str(buffer.str());
    original_metadata.id_ = original_request.id_ + "_meta";
    original_metadata.buffer_ = str.data();
    original_metadata.data_size_ = str.size();
    original_metadata.storage_index_ = primary_metadata.storage_index_;
    basket::Singleton<IOFactory>::GetInstance()->GetIOClient(original_metadata.storage_index_)->Write(original_metadata,original_metadata);

    if(!exists){
        auto link_metadata = Metadata();
        link_metadata.is_link_ = true;
        original_metadata.buffer_ = std::string().data();
        link_metadata.links_.insert({-1, original_metadata});
        auto link_meta = original_metadata;
        auto link_metadata_buf = std::make_shared<clmdep_msgpack::sbuffer>();
        clmdep_msgpack::pack(*link_metadata_buf, link_metadata);
        link_meta.buffer_ = link_metadata_buf->data();
        link_meta.data_size_ = link_metadata_buf->size();
        for(auto solution:SYMBIOS_CONF->STORAGE_SOLUTIONS) {
            bool is_primary = solution.first == original_request.storage_index_;
            if(!is_primary){
                link_meta.storage_index_ = solution.first;
                basket::Singleton<IOFactory>::GetInstance()->GetIOClient(link_meta.storage_index_)->Write(link_meta,link_meta);
            }
        }
    }
}

std::vector<Distribution> MetadataOrchestrator::Locate(Data &request, Metadata &primary_metadata) {

    Data original_metadata;
    original_metadata.id_ = request.id_ + "_meta";
    original_metadata.storage_index_=request.storage_index_;
    original_metadata.data_size_ = 0;
    basket::Singleton<IOFactory>::GetInstance()->GetIOClient(original_metadata.storage_index_)->Read(original_metadata,original_metadata);
    clmdep_msgpack::object_handle oh = clmdep_msgpack::unpack( (char*)original_metadata.buffer_,  original_metadata.data_size_);
    clmdep_msgpack::object deserialized = oh.get();
    Metadata metadata = deserialized.as<Metadata>();
    COMMON_DBGMSG("Deserialized");
    if(!metadata.is_link_){
        primary_metadata = metadata;
    }else{
        original_metadata = metadata.links_[-1];
        basket::Singleton<IOFactory>::GetInstance()->GetIOClient(original_metadata.storage_index_)->Read(original_metadata,original_metadata);
        clmdep_msgpack::object_handle oh = clmdep_msgpack::unpack( (char*)original_metadata.buffer_,  original_metadata.data_size_);
        oh.get().convert(primary_metadata);
    }
    auto distributions = std::vector<Distribution>();
    auto start_position = 0;
    for(auto link:primary_metadata.links_){
        if(link.first >= request.position_ && link.first <= request.position_ + request.data_size_){
            auto dest_data = link.second;
            if(request.position_ > link.first)
                dest_data.position_ = request.position_;
            if(request.position_ + request.data_size_ < link.first + link.second.data_size_)
                link.second.data_size_ = request.data_size_;
            auto distribution = Distribution();
            distribution.source_data_ = dest_data;
            distribution.destination_data_ = dest_data;
            distribution.storage_index_=distribution.destination_data_.storage_index_;
            distribution.source_data_.buffer_ = request.buffer_;
            distribution.source_data_.position_ = start_position;
            distributions.push_back(distribution);
        }
    }
    return distributions;
}

MetadataOrchestrator::MetadataOrchestrator() {

}
