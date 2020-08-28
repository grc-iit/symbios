//
// Created by mani on 8/24/2020.
//

#ifndef SYMBIOS_DP_DATA_PLACEMENT_H
#define SYMBIOS_DP_DATA_PLACEMENT_H

#include <symbios/data_placement/data_placement.h>

class DynamicProgrammingDDE: public DataDistributionEngine {
public:
    /*
     * Constructor
     */
    DynamicProgrammingDDE(){}
    /*
     * Methods
     */

    // Select the target storages for the request by using dynamic programming DDE
    std::vector<Distribution> Distribute(Data& request) override;
};

#endif //SYMBIOS_DP_DATA_PLACEMENT_H
