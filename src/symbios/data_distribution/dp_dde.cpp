//
// Created by mani on 8/24/2020.
//

#include <common/debug.h>
#include <string>
#include <symbios/data_distribution/dp_dde.h>

std::vector<DataDistribution> DynamicProgrammingDDE::Distribute(Data &request) {
    auto tracer = common::debug::AutoTrace("DynamicProgrammingDDE::Distribute", request);
    auto distributions = std::vector<DataDistribution>();
    COMMON_DBGVAR(distributions);
    return distributions;
}
