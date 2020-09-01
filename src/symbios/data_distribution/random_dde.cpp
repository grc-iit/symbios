//
// Created by mani on 8/24/2020.
//

#include <common/debug.h>
#include <cstdlib>
#include <ctime>
#include <symbios/common/configuration_manager.h>
#include <symbios/data_distribution/random_dde.h>

RandomDDE::RandomDDE() { srand((unsigned)SYMBIOS_CONF->RANDOM_SEED); }

std::vector<DataDistribution> RandomDDE::Distribute(Data &request) {

  auto tracer =
      common::debug::AutoTrace(std::string("RandomDDE::Distribute"), request);
  auto distributions = std::vector<DataDistribution>();
  auto random_number = rand();
  auto size = SYMBIOS_CONF->STORAGE_SOLUTIONS.size();
  int16_t selected_solution_index = random_number % size;
  auto selected_solution =
      SYMBIOS_CONF->STORAGE_SOLUTIONS[selected_solution_index];
  COMMON_DBGVAR3(random_number, size, selected_solution->io_client_type_);
  auto distribution = DataDistribution();
  distribution.storage_index_ = request.storage_index_;
  distribution.destination_data_ = request;
  distribution.source_data_ = request;
  distribution.destination_data_.storage_index_ = selected_solution_index;
  distributions.push_back(distribution);
  return distributions;
}
