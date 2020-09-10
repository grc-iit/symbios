//
// Created by mani on 8/24/2020.
//

#ifndef SYMBIOS_ROUNDROBINDATAPLACEMENT_H
#define SYMBIOS_ROUNDROBINDATAPLACEMENT_H

#include <symbios/data_distribution/data_distribution.h>
#include <basket/sequencer/global_sequence.h>

class RoundRobinDDE: public DataDistributionEngine {
private:
    basket::global_sequence sequence;
    uint16_t index;
public:
    /*
     * Constructor
     */
    RoundRobinDDE():sequence(),index(0){}
    /*
     * Methods
     */

    // Select target storages for the request by using round-robin data placement policy
    std::vector<DataDistribution> Distribute(Data &source, Data &destination) override;
};

#endif //SYMBIOS_ROUNDROBINDATAPLACEMENT_H
