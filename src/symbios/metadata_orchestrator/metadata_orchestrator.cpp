//
// Created by mani on 8/24/2020.
//

#include <common/debug.h>
#include <rpc/msgpack.hpp>
#include <symbios/common/configuration_manager.h>
#include <symbios/common/error_definition.h>
#include <symbios/io_clients/io_factory.h>
#include <symbios/metadata_orchestrator/metadata_orchestrator.h>

void MetadataOrchestrator::Store(Data &original_request,
                                 std::vector<DataDistribution> &distributions) {

    AUTO_TRACER("MetadataOrchestrator::Store", original_request, distributions);
    auto original_metadata = Data();
    /**
     * Update primary index
     */
    original_metadata.position_ = 0;
    original_metadata.storage_index_ = original_request.storage_index_;
    auto primary_metadata = Metadata();
    bool exists;
    auto existing_distributions = std::vector<DataDistribution>();
    try {
        existing_distributions = Locate(original_request, primary_metadata);
        exists = true;
    } catch (ErrorException e) {
        exists = false;
    }
    bool is_metadata_updated = false;
    if (exists) {
        for (auto &distribution : distributions) {
            auto segment = distribution.destination_data_.position_/ SYMBIOS_CONF->MAX_OBJ_SIZE;
            auto relative_segment_offset = distribution.destination_data_.position_ % SYMBIOS_CONF->MAX_OBJ_SIZE;
            auto iter = primary_metadata.links_.find(segment);
            if (iter == primary_metadata.links_.end()) {
                distribution.destination_data_.buffer_= NULL;
                distribution.destination_data_.position_ = relative_segment_offset;
                distribution.destination_data_.id_+=std::to_string(segment);
                primary_metadata.links_.insert({segment, distribution.destination_data_});
                is_metadata_updated = true;
            } else {
                distribution.destination_data_.id_ = iter->second.id_;
                distribution.destination_data_.position_ = relative_segment_offset;
                distribution.destination_data_.storage_index_ = iter->second.storage_index_;
            }
        }
    } else {
        is_metadata_updated = true;
        primary_metadata.is_link_ = false;
        primary_metadata.storage_index_ = original_request.storage_index_;
        for (auto &distribution:distributions) {
            auto segment = distribution.destination_data_.position_/ SYMBIOS_CONF->MAX_OBJ_SIZE;
            auto relative_segment_offset = distribution.destination_data_.position_ % SYMBIOS_CONF->MAX_OBJ_SIZE;

            distribution.destination_data_.position_ = relative_segment_offset;
            distribution.destination_data_.id_+=std::to_string(segment);
            auto link_data =  distribution.destination_data_;
            link_data.buffer_= NULL;
            primary_metadata.links_.insert({segment, link_data});
        }
    }
    original_metadata.id_ = original_request.id_ + "_meta";
    original_metadata.storage_index_ = primary_metadata.storage_index_;
    if (is_metadata_updated) {
        std::stringstream buffer;
        clmdep_msgpack::pack(buffer, primary_metadata);
        buffer.seekg(0);
        std::string s = buffer.str();
        original_metadata.data_size_ = s.size();
        original_metadata.buffer_ = static_cast<char *>(malloc(original_metadata.data_size_));
        memcpy(original_metadata.buffer_,s.c_str(),s.size());
        //original_metadata.buffer_=t;
        basket::Singleton<IOFactory>::GetInstance()->GetIOClient(original_metadata.storage_index_)->Write(
                original_metadata, original_metadata);
        free(original_metadata.buffer_);
        original_metadata.buffer_=NULL;
    }
    if (!exists) {
        auto link_metadata = Metadata();
        link_metadata.is_link_ = true;
        link_metadata.links_.insert({-1, original_metadata});
        auto link_meta = Data();
        link_meta.id_=original_request.id_ + "_meta";
        std::stringstream buffer;
        clmdep_msgpack::pack(buffer, link_metadata);
        buffer.seekg(0);
        std::string s = buffer.str();
        link_meta.data_size_ = s.size();
        link_meta.buffer_ = static_cast<char *>(malloc(link_meta.data_size_));
        memcpy(link_meta.buffer_,s.data(),link_meta.data_size_);
        for (auto &solution:SYMBIOS_CONF->STORAGE_SOLUTIONS) {
            bool is_primary = solution.first == original_request.storage_index_;
            if (!is_primary) {
                link_meta.storage_index_ = solution.first;
                basket::Singleton<IOFactory>::GetInstance()->GetIOClient(link_meta.storage_index_)->Write(link_meta,
                                                                                                          link_meta);
            }
        }
        free(link_meta.buffer_);
        link_meta.buffer_=NULL;
    }
}


std::vector<DataDistribution>
MetadataOrchestrator::Locate(Data &request, Metadata &primary_metadata) {
    AUTO_TRACER("MetadataOrchestrator::Locate", request, primary_metadata);
    Data original_metadata;
    original_metadata.id_ = request.id_ + "_meta";
    original_metadata.storage_index_ = request.storage_index_;
    basket::Singleton<IOFactory>::GetInstance()->GetIOClient(original_metadata.storage_index_)->Read(original_metadata,
                                                                                                     original_metadata);
    std::string s = std::string(original_metadata.buffer_,original_metadata.data_size_);
    //std::replace( s.begin(), s.end(), '$','\0');
    clmdep_msgpack::object_handle oh = clmdep_msgpack::unpack(s.c_str(),
                                                              original_metadata.data_size_);
    clmdep_msgpack::object deserialized = oh.get();
    Metadata metadata = deserialized.as<Metadata>();
    free(original_metadata.buffer_);
    original_metadata.buffer_=NULL;
    COMMON_DBGMSG("Deserialized");
    if (!metadata.is_link_) {
        primary_metadata = metadata;
    } else {
        original_metadata = metadata.links_[-1];
        basket::Singleton<IOFactory>::GetInstance()->GetIOClient(original_metadata.storage_index_)->Read(
                original_metadata, original_metadata);
        std::string s = std::string(original_metadata.buffer_,original_metadata.data_size_);
        //std::replace( s.begin(), s.end(), '$','\0');
        clmdep_msgpack::object_handle oh = clmdep_msgpack::unpack(s.c_str(),
                                                                  original_metadata.data_size_);
        oh.get().convert(primary_metadata);
        free(original_metadata.buffer_);
        original_metadata.buffer_=NULL;
        COMMON_DBGMSG("Getting Primary metadata using hop");
    }
    auto distributions = std::vector<DataDistribution>();
    bool get_all = request.data_size_ == 0;
    auto start_segment = request.position_/SYMBIOS_CONF->MAX_OBJ_SIZE;
    auto end_segment = (request.position_+request.data_size_)/SYMBIOS_CONF->MAX_OBJ_SIZE;
    Data links[primary_metadata.links_.size()];
    auto link_size = primary_metadata.links_.size();
    for (auto &link:primary_metadata.links_) {
        links[link.first]=link.second;
    }
    auto start_offset = request.position_;
    auto size_left = request.data_size_;
    auto destination_position = 0;
    for(int i=start_segment; (get_all || i <= end_segment) && i < link_size; ++i){
        auto relative_offset =  get_all ? 0:start_offset % SYMBIOS_CONF->MAX_OBJ_SIZE;
        auto source = links[i];
        auto destination = links[i];
        source.position_=relative_offset;
        source.data_size_=SYMBIOS_CONF->MAX_OBJ_SIZE-relative_offset > size_left?size_left:SYMBIOS_CONF->MAX_OBJ_SIZE-relative_offset;
        destination.position_ = destination_position;
        destination.data_size_ = source.data_size_;
        start_offset += source.data_size_;
        destination_position += source.data_size_;
        size_left-=source.data_size_;
        auto distribution = DataDistribution();
        distribution.source_data_ = source;
        distribution.destination_data_ = destination;
        distribution.storage_index_ = source.storage_index_;
        distributions.push_back(distribution);
    }
    COMMON_DBGVAR(distributions);
    return distributions;
}

MetadataOrchestrator::MetadataOrchestrator() {}

bool MetadataOrchestrator::Delete(Data &request) {
    for (auto solution:SYMBIOS_CONF->STORAGE_SOLUTIONS) {
        Data original_metadata;
        original_metadata.id_ = request.id_ + "_meta";
        original_metadata.storage_index_ = solution.first;
        basket::Singleton<IOFactory>::GetInstance()->GetIOClient(original_metadata.storage_index_)->Remove(original_metadata);
    }
    return true;
}
