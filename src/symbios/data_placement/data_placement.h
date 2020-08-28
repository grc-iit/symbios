//
// Created by mani on 8/24/2020.
//

#ifndef SYMBIOS_DATA_PLACEMENT_H
#define SYMBIOS_DATA_PLACEMENT_H

#include <symbios/common/data_structure.h>

class DataDistributionEngine {
public:
    /*
     * Constructor
     */
    DataDistributionEngine(){}
    /*
     * Methods
     */

    // select the target storage
    //
    virtual void Distribute(Data& request) = 0;
};

#endif //SYMBIOS_DATA_PLACEMENT_H
