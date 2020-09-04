//
// Created by mani on 8/24/2020.
//

#include <common/debug.h>
#include <symbios/common/configuration_manager.h>
#include <symbios/data_distribution/round_robin_dde.h>

std::vector<DataDistribution> RoundRobinDDE::Distribute(Data &request) {
    AUTO_TRACER("RoundRobinDDE::Distribute", request);
    auto distributions = std::vector<DataDistribution>();

    auto next_value = index++;//sequence.GetNextSequenceServer(BASKET_CONF->MY_SERVER);
    int16_t selected_solution_index = next_value % SYMBIOS_CONF->STORAGE_SOLUTIONS.size();
    COMMON_DBGVAR(next_value);
    auto selected_solution = SYMBIOS_CONF->STORAGE_SOLUTIONS[selected_solution_index];
    auto distribution = DataDistribution();
    distribution.storage_index_ = request.storage_index_;
    distribution.destination_data_ = request;
    distribution.source_data_ = request;
    distribution.destination_data_.storage_index_ = selected_solution_index;
    distributions.push_back(distribution);
    COMMON_DBGVAR(distributions);
    return distributions;
}
