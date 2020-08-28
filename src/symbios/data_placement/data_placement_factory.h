//
// Created by mani on 8/24/2020.
//

#ifndef SYMBIOS_DATA_PLACEMENT_FACTORY_H
#define SYMBIOS_DATA_PLACEMENT_FACTORY_H

#include <symbios/data_placement/data_placement.h>
#include <symbios/common/enumerations.h>
#include <symbios/data_placement/random_data_placement.h>
#include <symbios/data_placement/round_robin_data_placement.h>
#include <symbios/data_placement/heuristics_data_placement.h>
#include <symbios/data_placement/dp_data_placement.h>

class DataDistributionEngineFactory {
public:
    DataDistributionEngineFactory(){}

    std::shared_ptr<DataDistributionEngine> GetDataDistributionEngine(DataPlacementPolicy &policy){
        switch (policy){
            case DataPlacementPolicy::RANDOM_POLICY:
                return Singleton<RandomDDE>::GetInstance();
            case DataPlacementPolicy::ROUND_ROBIN_POLICY:
                return Singleton<RoundRobinDDE>::GetInstance();
            case DataPlacementPolicy::HEURISTICS_POLICY:
                return Singleton<HeuristicsDDE>::GetInstance();
            case DataPlacementPolicy::DYNAMIC_PROGRAMMING_POLICY:
                return Singleton<DynamicProgrammingDDE>::GetInstance();

        }
    }
};

#endif //SYMBIOS_DATA_PLACEMENT_FACTORY_H
