//
// Created by mani on 8/24/2020.
//

#ifndef SYMBIOS_HEURISTICS_DDE_H
#define SYMBIOS_HEURISTICS_DDE_H

#include <symbios/data_distribution/data_distribution.h>

class HeuristicsDDE: public DataDistributionEngine {
private:
    std::unordered_map<IOClientType, std::pair<uint16_t,std::shared_ptr<StorageSolution>>> maps;
public:
    /*
     * Constructor
     */
    HeuristicsDDE();
    /*
     * Methods
     */

    // Select the target storages for the request by using Heuristics data placement policy
    std::vector<DataDistribution> Distribute(Data& request) override;
};

#endif //SYMBIOS_HEURISTICS_DDE_H
