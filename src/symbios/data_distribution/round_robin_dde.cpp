//
// Created by mani on 8/24/2020.
//

#include <symbios/data_distribution/round_robin_dde.h>
#include <symbios/common/configuration_manager.h>

std::vector<Distribution> RoundRobinDDE::Distribute(Data& request) {

    auto distributions = std::vector<Distribution>();
    uint16_t server_index = 0;
    auto next_value = sequence.GetNextSequenceServer(server_index);
    int16_t selected_solution_index = rand() % SYMBIOS_CONF->STORAGE_SOLUTIONS.size();
    auto selected_solution = SYMBIOS_CONF->STORAGE_SOLUTIONS[selected_solution_index];
    auto distribution = Distribution();
    distribution.io_client_type_ = request.io_client_type_;
    distribution.destination_data_ = request;
    distribution.destination_data_ = request;
    distribution.destination_data_.io_client_type_ = selected_solution->io_client_type_;
    distributions.push_back(distribution);
    return distributions;
}