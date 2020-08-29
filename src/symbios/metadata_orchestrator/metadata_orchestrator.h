//
// Created by mani on 8/24/2020.
//

#ifndef SYMBIOS_METADATA_ORCHESTRATOR_H
#define SYMBIOS_METADATA_ORCHESTRATOR_H


#include <symbios/common/data_structure.h>

class MetadataOrchestrator {
public:
    void Store(Data &original_request, std::vector<Distribution> &distributions);
    std::vector<Distribution> Locate(Data &request);
};


#endif //SYMBIOS_METADATA_ORCHESTRATOR_H
