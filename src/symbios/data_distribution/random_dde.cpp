//
// Created by mani on 8/24/2020.
//

#include <symbios/data_distribution/random_dde.h>
#include <cstdlib>
#include <ctime>

RandomDDE::RandomDDE() {
    //srand((unsigned)time(NULL));
}

std::vector<Distribution> RandomDDE::Distribute(Data& request) {
    // get storage vector from the configurationManager

    //int size = 5;
    //int index = rand() % size;
    // get the target storage in vector[index], and then return it.

    return std::vector<Distribution>();
}

