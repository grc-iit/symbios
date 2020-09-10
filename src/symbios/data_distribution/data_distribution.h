//
// Created by mani on 8/24/2020.
//

#ifndef SYMBIOS_DATA_DISTRIBUTION_H
#define SYMBIOS_DATA_DISTRIBUTION_H

#include <symbios/common/data_structure.h>
#include <vector>

/*
 * An abstract class which provides data distribution interface for other modules
 */
class DataDistributionEngine {
public:
    /*
     * Constructor
     */
    DataDistributionEngine(){}
    /*
     * Methods
     */

    // select the target storages for the source request
    virtual std::vector<DataDistribution> Distribute(Data &source, Data &destination) = 0;
};

#endif //SYMBIOS_DATA_DISTRIBUTION_H
