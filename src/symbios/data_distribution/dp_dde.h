//
// Created by mani on 8/24/2020.
//

#ifndef SYMBIOS_DP_DDE_H
#define SYMBIOS_DP_DDE_H

#include <symbios/data_distribution/data_distribution.h>

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

#endif //SYMBIOS_DP_DDE_H
