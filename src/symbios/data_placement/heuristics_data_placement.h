//
// Created by mani on 8/24/2020.
//

#ifndef SYMBIOS_HEURISTICS_DATA_PLACEMENT_H
#define SYMBIOS_HEURISTICS_DATA_PLACEMENT_H

#include <symbios/data_placement/data_placement.h>

class HeuristicsDDE: public DataDistributionEngine {
public:
    /*
     * Constructor
     */
    HeuristicsDDE(){}
    /*
     * Methods
     */

    // Select the target storages for the request by using Heuristics data placement policy
    std::vector<Distribution> Distribute(Data& request) override;
};

#endif //SYMBIOS_HEURISTICS_DATA_PLACEMENT_H
