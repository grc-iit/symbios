//
// Created by mani on 8/24/2020.
//

#include <common/debug.h>
#include <symbios/common/configuration_manager.h>
#include <symbios/data_distribution/heuristics_dde.h>

/*
 * Default Constructor
 * 1) initialize the maps according to the Storage Solutions we configured in configuration file
 */
HeuristicsDDE::HeuristicsDDE() : maps() {
    for (auto entry : SYMBIOS_CONF->STORAGE_SOLUTIONS) {
        maps.insert({entry.second->io_client_type_,
                     std::pair<uint16_t, std::shared_ptr<StorageSolution>>(
                             entry.first, entry.second)});
    }
}

/*
 * Select target data distributions for the request by using heuristics policy
 * 1) if request data size is less than 16KB, Selecting the Mongo io
 * 2) if request data size is greater than or equal to 16KB but less than 128KB, Selecting the Redis io
 * 3) if request data size is greater than or equal to 128KB, Selecting the file io.
 * @Parameter request: the source request data information
 * @Parameter destination: the original destination data information
 * @return std::vector<DataDistribution>: return a group of selected data distributions
 */
std::vector<DataDistribution> HeuristicsDDE::Distribute(Data &source, Data &destination) {
    AUTO_TRACER("HeuristicsDDE::Distribute", request);
    auto distributions = std::vector<DataDistribution>();
    int16_t selected_solution_index = 0;
    COMMON_DBGVAR(request.data_size_);
    if (destination.data_size_ < 16 * 1024) {
        selected_solution_index = maps[IOClientType::MONGO_IO].first;
    } else if (destination.data_size_ >= 16 * 1024 and destination.data_size_ < 128 * 1024) {
        selected_solution_index = maps[IOClientType::REDIS_IO].first;
    } else {
        selected_solution_index = maps[IOClientType::FILE_IO].first;
    }
    COMMON_DBGVAR(selected_solution_index);
    auto selected_solution = SYMBIOS_CONF->STORAGE_SOLUTIONS[selected_solution_index];
    auto distribution = DataDistribution();
    distribution.storage_index_ = source.storage_index_;
    distribution.destination_data_ = destination;
    distribution.source_data_ = source;
    distribution.destination_data_.storage_index_ = selected_solution_index;
    distributions.push_back(distribution);
    COMMON_DBGVAR(distributions);
    return distributions;
}
