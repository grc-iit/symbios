//
// Created by mani on 8/24/2020.
//

#ifndef SYMBIOS_ROUNDROBINDATAPLACEMENT_H
#define SYMBIOS_ROUNDROBINDATAPLACEMENT_H

#include <symbios/data_placement/data_placement.h>

class RoundRobinDDE: public DataDistributionEngine {
public:
    /*
     * Constructor
     */
    RoundRobinDDE(){}
    /*
     * Methods
     */

    // Select target storages for the request by using round-robin data placement policy
    std::vector<Distribution> Distribute(Data& request) override;
};

#endif //SYMBIOS_ROUNDROBINDATAPLACEMENT_H
