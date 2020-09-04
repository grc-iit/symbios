//
// Created by mani on 8/24/2020.
//

#ifndef SYMBIOS_DATA_DISTRIBUTION_FACTORY_H
#define SYMBIOS_DATA_DISTRIBUTION_FACTORY_H

#include <symbios/data_distribution/data_distribution.h>
#include <symbios/common/enumerations.h>
#include <symbios/data_distribution/random_dde.h>
#include <symbios/data_distribution/round_robin_dde.h>
#include <symbios/data_distribution/heuristics_dde.h>
#include <symbios/data_distribution/dp_dde.h>
#include <basket/common/singleton.h>
#include <common/debug.h>

class DataDistributionEngineFactory {
public:
    DataDistributionEngineFactory() {}

    std::shared_ptr<DataDistributionEngine> GetDataDistributionEngine(DataDistributionPolicy &policy) {
        AUTO_TRACER("DataDistributionEngineFactory::GetDataDistributionEngine", policy);
        switch (policy) {
            case DataDistributionPolicy::RANDOM_POLICY:
                return basket::Singleton<RandomDDE>::GetInstance();
            case DataDistributionPolicy::ROUND_ROBIN_POLICY:
                return basket::Singleton<RoundRobinDDE>::GetInstance();
            case DataDistributionPolicy::HEURISTICS_POLICY:
                return basket::Singleton<HeuristicsDDE>::GetInstance();
            case DataDistributionPolicy::DYNAMIC_PROGRAMMING_POLICY:
                return basket::Singleton<DynamicProgrammingDDE>::GetInstance();

        }
    }
};

#endif //SYMBIOS_DATA_DISTRIBUTION_FACTORY_H
