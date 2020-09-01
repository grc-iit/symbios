//
// Created by mani on 8/24/2020.
//

#include <common/debug.h>
#include <symbios/common/configuration_manager.h>
#include <symbios/data_distribution/round_robin_dde.h>

std::vector<DataDistribution> RoundRobinDDE::Distribute(Data &request) {
  auto tracer = common::debug::AutoTrace(
      std::string("RoundRobinDDE::Distribute"), request);
  auto distributions = std::vector<DataDistribution>();
  uint16_t index = 0;
  auto next_value = sequence.GetNextSequence();
  int16_t selected_solution_index =
      next_value % SYMBIOS_CONF->STORAGE_SOLUTIONS.size();
  auto selected_solution =
      SYMBIOS_CONF->STORAGE_SOLUTIONS[selected_solution_index];
  auto distribution = DataDistribution();
  distribution.storage_index_ = request.storage_index_;
  distribution.destination_data_ = request;
  distribution.source_data_ = request;
  distribution.destination_data_.storage_index_ = selected_solution_index;
  distributions.push_back(distribution);
  COMMON_DBGVAR((char *)request.buffer_);
  return distributions;
}
