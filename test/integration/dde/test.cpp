//
// Created by mani on 8/30/2020.
//

#include <common/arguments.h>
#include <symbios/common/enumerations.h>
#include <mpi.h>
#include <symbios/common/configuration_manager.h>
#include <basket/common/macros.h>
#include <common/rng.h>
#include <symbios/data_distribution/data_distribution_factory.h>
#include <common/debug.h>
#include <boost/filesystem.hpp>


class BenchmarkArgs : public ArgMap {
private:
    void VerifyArgs(void) {
        AssertOptIsSet("-p");
        int dde = GetIntOpt("-p");
        switch(static_cast<DataDistributionPolicy>(dde)) {
            case DataDistributionPolicy::RANDOM_POLICY: {
                AssertOptIsSet("-seed");
                break;
            }
            case DataDistributionPolicy::ROUND_ROBIN_POLICY:
            case DataDistributionPolicy::HEURISTICS_POLICY:
            case DataDistributionPolicy::DYNAMIC_PROGRAMMING_POLICY: {
            }
        }
        AssertOptIsSet("-s");
        AssertOptIsSet("-n");
        AssertOptIsSet("-c");
        AssertOptIsSet("-out");


    }

public:
    void Usage(void) {
        std::cout << "Usage: ./dde -[param-id] [value] ... " << std::endl;
        std::cout << std::endl;
        std::cout << "-c [string]: path to configuration file" << std::endl;
        std::cout << "-s [int]: size of request" << std::endl;
        std::cout << "-n [int]: number of requests" << std::endl;
        std::cout << "-p [string]: Which dde policy to run" << std::endl;
        std::cout << "   RANDOM_POLICY" << std::endl;
        std::cout << "   ROUND_ROBIN_POLICY" << std::endl;
        std::cout << "   HEURISTICS_POLICY" << std::endl;
        std::cout << "   DYNAMIC_PROGRAMMING_POLICY" << std::endl;
        std::cout << "Random DDE Parameters" << std::endl;
        std::cout << "-seed [int]" << std::endl;
        std::cout << "Additional Parameters" << std::endl;
        std::cout << "-out [string]: path to csv file" << std::endl;
        std::cout << "" << std::endl;
    }

    BenchmarkArgs(int argc, char **argv) {
        AddOpt("-p", ArgType::kStringMap);
        AddStringMapVal("-p", "RANDOM_POLICY", static_cast<int>(DataDistributionPolicy::RANDOM_POLICY));
        AddStringMapVal("-p", "ROUND_ROBIN_POLICY", static_cast<int>(DataDistributionPolicy::ROUND_ROBIN_POLICY));
        AddStringMapVal("-p", "HEURISTICS_POLICY", static_cast<int>(DataDistributionPolicy::HEURISTICS_POLICY));
        AddStringMapVal("-p", "DYNAMIC_PROGRAMMING_POLICY", static_cast<int>(DataDistributionPolicy::DYNAMIC_PROGRAMMING_POLICY));
        AddOpt("-c", ArgType::kString);
        AddOpt("-out", ArgType::kString);
        AddOpt("-s", ArgType::kInt);
        AddOpt("-n", ArgType::kInt);
        AddOpt("-seed", ArgType::kInt);
        ArgIter(argc, argv);
        VerifyArgs();
    }
};

int main(int argc, char* argv[]){
    MPI_Init(&argc,&argv);
    BenchmarkArgs args(argc, argv);
    int rank, nprocs = 1;
    int ops_per_proc = 0;
    size_t bytes_per_proc = 0;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    std::string config = args.GetStringOpt("-c");
    SYMBIOS_CONF->CONFIGURATION_FILE=config.data();
    BASKET_CONF->BACKED_FILE_DIR=SYMBIOS_CONF->SERVER_DIR;
    SYMBIOS_CONF->ConfigureSymbiosServer();
    int dde_i = args.GetIntOpt("-p");
    SYMBIOS_CONF->DATA_DISTRIBUTION_POLICY = static_cast<DataDistributionPolicy>(dde_i);
    SYMBIOS_CONF->RANDOM_SEED = args.GetIntOpt("-seed");

    auto basket = BASKET_CONF;
    int request_size = args.GetIntOpt("-s");
    int number_request = args.GetIntOpt("-n");
    auto distribution = DistributionFactory::Get(DistributionType::kUniform);
    distribution->Shape((double)request_size/sizeof(double));

    auto engine = basket::Singleton<DataDistributionEngineFactory>::GetInstance()->GetDataDistributionEngine(SYMBIOS_CONF->DATA_DISTRIBUTION_POLICY);
    auto request = Data();
    request.position_ = 0;
    request.storage_index_ = 0;
    request.buffer_ = distribution.get();
    request.data_size_ = request_size;
    common::debug::Timer t;
    ops_per_proc = number_request;
    bytes_per_proc = ops_per_proc * request_size;
    for(int i=0;i<number_request;++i){
        request.id_ = "temp_" + std::to_string(i);
        t.resumeTime();
        engine->Distribute(request);
        t.pauseTime();
    }
    double local_end_time = t.endTime();
    double local_std_msec;

    //Get global statistics
    double avg_msec, std_msec, min_msec, max_msec;
    MPI_Allreduce(&local_end_time, &avg_msec, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    local_std_msec = pow(local_end_time - avg_msec, 2);
    MPI_Reduce(&local_std_msec, &std_msec, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_end_time, &min_msec, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_end_time, &max_msec, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    avg_msec = avg_msec/nprocs;
    std_msec = std::sqrt(std_msec);

    //Get average bandwidth and throughput
    size_t tot_ops = ((size_t)ops_per_proc) * nprocs;
    size_t tot_bytes = bytes_per_proc * nprocs;
    double thrpt_kiops = ((double)tot_ops)/avg_msec;
    double bw_kbps = ((double)tot_bytes)/avg_msec;

    //Write to output CSV in root process
    if(rank == 0 && args.OptIsSet("-out")) {
        std::string output_path = args.GetStringOpt("-out");
        bool exists = boost::filesystem::exists(output_path);
        std::ofstream out(output_path, std::ofstream::out | std::ofstream::app);
        if(!exists) {
            out << "avg_msec,std_msec,min_msec,max_msec,nprocs,number_request,request_size,seed,tot_ops,tot_bytes,thrpt_kiops,bw_kbps,dde" << std::endl;
        }
        out <<
            avg_msec << "," <<
            std_msec << "," <<
            min_msec << "," <<
            max_msec << "," <<
            nprocs << "," <<
            number_request << "," <<
            request_size << "," <<
            SYMBIOS_CONF->RANDOM_SEED << "," <<
            tot_ops << "," <<
            tot_bytes << "," <<
            thrpt_kiops << "," <<
            bw_kbps << "," <<
            dde_i << std::endl;
    }

    MPI_Finalize();


}
