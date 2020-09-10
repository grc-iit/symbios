//
// Created by mani on 8/24/2020.
//

#include <common/debug.h>
#include <string>
#include <symbios/data_distribution/dp_dde.h>

/*
 * Select target data distributions for the source request by using dynamic programming policy
 * @Parameter source: the source request data information
 * @Parameter destination: the original destination data information
 * @return std::vector<DataDistribution>: return a group of selected data distributions
 */
std::vector<DataDistribution> DynamicProgrammingDDE::Distribute(Data &source, Data &destination) {
    AUTO_TRACER("DynamicProgrammingDDE::Distribute", request);
    auto distributions = std::vector<DataDistribution>();
    COMMON_DBGVAR(distributions);
    return distributions;
}
