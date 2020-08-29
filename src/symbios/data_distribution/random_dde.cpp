//
// Created by mani on 8/24/2020.
//

#include <symbios/data_distribution/random_dde.h>
#include <cstdlib>
#include <ctime>
#include <symbios/common/configuration_manager.h>

RandomDDE::RandomDDE() {
    srand((unsigned) SYMBIOS_CONF->RANDOM_SEED);
}

std::vector<Distribution> RandomDDE::Distribute(Data& request) {

    auto distributions = std::vector<Distribution>();
    int16_t selected_solution_index = rand() % SYMBIOS_CONF->STORAGE_SOLUTIONS.size();
    auto selected_solution = SYMBIOS_CONF->STORAGE_SOLUTIONS[selected_solution_index];
    auto distribution = Distribution();
    distribution.io_client_type_ = request.io_client_type_;
    distribution.source_data_ = request;
    distribution.destination_data_ = request;
    distribution.io_client_type_ = distribution.io_client_type_;
    distributions.push_back(distribution);
    return distributions;
}

