//
// Created by mani on 8/24/2020.
//

#ifndef SYMBIOS_RANDOMDATAPLACEMENT_H
#define SYMBIOS_RANDOMDATAPLACEMENT_H

#include <symbios/data_distribution/data_distribution.h>

/*
 * A subclass which inherits from DataDistributionEngine
 * 1) selecting target data distributions by random policy
 */
class RandomDDE: public DataDistributionEngine {
public:
    /*
     * Constructor
     */
    RandomDDE();
    /*
     * Methods
     */

    // Select the target storages for the source request by using random data placement policy
    std::vector<DataDistribution> Distribute(Data &source, Data &destination) override;
};

#endif //SYMBIOS_RANDOMDATAPLACEMENT_H
