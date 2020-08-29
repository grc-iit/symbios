//
// Created by mani on 8/24/2020.
//

#include <symbios/metadata_orchestrator/metadata_orchestrator.h>
#include <symbios/common/configuration_manager.h>

void MetadataOrchestrator::Store(Data &original_request, std::vector<Distribution> &distributions) {
    for(auto solution:SYMBIOS_CONF->STORAGE_SOLUTIONS){
        if()
    }
}

std::vector<Distribution> MetadataOrchestrator::Locate(Data &request) {


    return std::vector<Distribution>();
}

MetadataOrchestrator::MetadataOrchestrator() {

}
