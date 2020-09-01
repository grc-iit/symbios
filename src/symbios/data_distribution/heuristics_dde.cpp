//
// Created by mani on 8/24/2020.
//

#include <common/debug.h>
#include <symbios/common/configuration_manager.h>
#include <symbios/data_distribution/heuristics_dde.h>

HeuristicsDDE::HeuristicsDDE() : maps() {
  for (auto entry : SYMBIOS_CONF->STORAGE_SOLUTIONS) {
    maps.insert({entry.second->io_client_type_,
                 std::pair<uint16_t, std::shared_ptr<StorageSolution>>(
                     entry.first, entry.second)});
  }
}

std::vector<DataDistribution> HeuristicsDDE::Distribute(Data &request) {
  auto tracer = common::debug::AutoTrace(
      std::string("HeuristicsDDE::Distribute"), request);
  auto distributions = std::vector<DataDistribution>();
  int16_t selected_solution_index = 0;

    if(request.buffer_.size() < 16*1024){
        selected_solution_index = maps[IOClientType::MONGO_IO].first;
    }else if(request.buffer_.size() >= 16*1024 and request.buffer_.size() < 128*1024){
        selected_solution_index = maps[IOClientType::REDIS_IO].first;
    }else{
        selected_solution_index = maps[IOClientType::FILE_IO].first;
    }
    auto selected_solution = SYMBIOS_CONF->STORAGE_SOLUTIONS[selected_solution_index];
    auto distribution = DataDistribution();
    distribution.storage_index_ = request.storage_index_;
    distribution.destination_data_ = request;
    distribution.source_data_ = request;
    distribution.destination_data_.storage_index_ = selected_solution_index;
    distributions.push_back(distribution);
    return distributions;
}
