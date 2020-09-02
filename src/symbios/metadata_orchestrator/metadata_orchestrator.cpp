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

    auto tracer = common::debug::AutoTrace("MetadataOrchestrator::Store", original_request, distributions);
    auto original_metadata = original_request;
    /**
     * Update primary index
     */
    original_metadata.position_ = 0;
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
        for (auto distribution : distributions) {
            auto iter = primary_metadata.links_.find(distribution.destination_data_.position_);
            if (iter == primary_metadata.links_.end()) {
                distribution.destination_data_.buffer_ = std::string().data();
                primary_metadata.links_.insert(
                        {distribution.destination_data_.position_, distribution.destination_data_});
                is_metadata_updated = true;
            } else {
                distribution.destination_data_.storage_index_ = iter->second.storage_index_;
            }
        }
    } else {
        is_metadata_updated = true;
        primary_metadata.is_link_ = false;
        primary_metadata.storage_index_ = original_request.storage_index_;
        for (auto distribution:distributions) {
            auto link_data =  distribution.destination_data_;
            link_data.buffer_ = std::string().data();
            link_data.id_ = original_request.id_ + "_meta";
            link_data.position_=0;
            primary_metadata.links_.insert({distribution.destination_data_.position_, link_data});
        }
    }
    if (is_metadata_updated) {
        std::stringstream buffer;
        clmdep_msgpack::pack(buffer, primary_metadata);
        buffer.seekg(0);
        original_metadata.id_ = original_request.id_ + "_meta";
        original_metadata.buffer_ = buffer.str();
        original_metadata.storage_index_ = primary_metadata.storage_index_;
        basket::Singleton<IOFactory>::GetInstance()->GetIOClient(original_metadata.storage_index_)->Write(
                original_metadata, original_metadata);
    }
    if (!exists) {
        auto link_metadata = Metadata();
        link_metadata.is_link_ = true;
        original_metadata.buffer_ = std::string().data();
        link_metadata.links_.insert({-1, original_metadata});
        auto link_meta = original_metadata;
        std::stringstream link_metadata_buf;
        clmdep_msgpack::pack(link_metadata_buf, link_metadata);
        link_metadata_buf.seekg(0);
        link_meta.buffer_ = link_metadata_buf.str();
        for (auto solution:SYMBIOS_CONF->STORAGE_SOLUTIONS) {
            bool is_primary = solution.first == original_request.storage_index_;
            if (!is_primary) {
                link_meta.storage_index_ = solution.first;
                basket::Singleton<IOFactory>::GetInstance()->GetIOClient(link_meta.storage_index_)->Write(link_meta,
                                                                                                          link_meta);
            }
        }
    }
}


std::vector<DataDistribution>
MetadataOrchestrator::Locate(Data &request, Metadata &primary_metadata) {
    auto tracer = common::debug::AutoTrace("MetadataOrchestrator::Locate", request, primary_metadata);
    Data original_metadata;
    original_metadata.id_ = request.id_ + "_meta";
    original_metadata.storage_index_ = request.storage_index_;
    basket::Singleton<IOFactory>::GetInstance()->GetIOClient(original_metadata.storage_index_)->Read(original_metadata,
                                                                                                     original_metadata);
    clmdep_msgpack::object_handle oh = clmdep_msgpack::unpack(original_metadata.buffer_.data(),
                                                              original_metadata.buffer_.size());
    clmdep_msgpack::object deserialized = oh.get();
    Metadata metadata = deserialized.as<Metadata>();
    COMMON_DBGMSG("Deserialized");
    if (!metadata.is_link_) {
        primary_metadata = metadata;
    } else {
        original_metadata = metadata.links_[-1];
        basket::Singleton<IOFactory>::GetInstance()->GetIOClient(original_metadata.storage_index_)->Read(
                original_metadata, original_metadata);
        clmdep_msgpack::object_handle oh = clmdep_msgpack::unpack(original_metadata.buffer_.data(),
                                                                  original_metadata.buffer_.size());
        oh.get().convert(primary_metadata);
        COMMON_DBGMSG("Getting Primary metadata using hop");
    }
    auto distributions = std::vector<DataDistribution>();
    auto start_position = 0;
    bool get_all = request.buffer_.size() == 0;
    for (auto link:primary_metadata.links_) {
        if (get_all || link.first >= request.position_ && link.first <= request.buffer_.size()) {
            auto dest_data = link.second;
            if (!get_all) {
                if (request.position_ > link.first)
                    dest_data.position_ = request.position_;
                if (request.buffer_.size() < link.first + link.second.buffer_.size() - link.second.position_)
                    link.second.buffer_ = request.buffer_;
            }
            auto distribution = DataDistribution();
            distribution.source_data_ = dest_data;
            distribution.destination_data_ = dest_data;
            distribution.storage_index_ = distribution.destination_data_.storage_index_;
            distribution.source_data_.buffer_ = request.buffer_;
            distribution.source_data_.position_ = start_position;
            distributions.push_back(distribution);
        }
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
