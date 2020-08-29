//
// Created by mani on 8/24/2020.
//

#include <symbios/data_distribution/random_dde.h>
#include <cstdlib>
#include <ctime>
#include <symbios/common/configuration_manager.h>
#include <common/debug.h>


RandomDDE::RandomDDE() {
    srand((unsigned) SYMBIOS_CONF->RANDOM_SEED);
}

std::vector<Distribution> RandomDDE::Distribute(Data& request) {

    auto distributions = std::vector<Distribution>();
    auto random_number = rand();
    auto size = SYMBIOS_CONF->STORAGE_SOLUTIONS.size();
    int16_t selected_solution_index = random_number % size;
    auto selected_solution = SYMBIOS_CONF->STORAGE_SOLUTIONS[selected_solution_index];
    COMMON_DBGVAR3(random_number,size,selected_solution->io_client_type_);
    auto distribution = Distribution();
    distribution.io_client_type_ = request.io_client_type_;
    distribution.destination_data_ = request;
    distribution.source_data_ = std::move(request);
    distribution.destination_data_.io_client_type_ = selected_solution->io_client_type_;
    distributions.push_back(distribution);
    return distributions;
}

