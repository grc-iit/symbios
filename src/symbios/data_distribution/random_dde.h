//
// Created by mani on 8/24/2020.
//

#ifndef SYMBIOS_RANDOMDATAPLACEMENT_H
#define SYMBIOS_RANDOMDATAPLACEMENT_H

#include <symbios/data_distribution/data_distribution.h>

class RandomDDE: public DataDistributionEngine {
public:
    /*
     * Constructor
     */
    RandomDDE();
    /*
     * Methods
     */

    // Select the target storages for the request by using random data placement policy
    std::vector<DataDistribution> Distribute(Data &source, Data &destination) override;
};

#endif //SYMBIOS_RANDOMDATAPLACEMENT_H
