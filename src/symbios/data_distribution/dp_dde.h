//
// Created by mani on 8/24/2020.
//

#ifndef SYMBIOS_DP_DDE_H
#define SYMBIOS_DP_DDE_H

#include <symbios/data_distribution/data_distribution.h>

/*
 * A subclass which inherits from DataDistributionEngine
 * 1) selecting target data distributions by dynamic programming policy
 */
class DynamicProgrammingDDE: public DataDistributionEngine {
public:
    /*
     * Constructor
     */
    DynamicProgrammingDDE(){}
    /*
     * Methods
     */

    // Select the target storages for the source request by using dynamic programming DDE
    std::vector<DataDistribution> Distribute(Data &source, Data &destination) override;
};

#endif //SYMBIOS_DP_DDE_H
