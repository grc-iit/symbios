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
#include <symbios/metadata_orchestrator/metadata_orchestrator.h>


class BenchmarkArgs : public common::args::ArgMap {
private:
    void VerifyArgs(void) {
        AssertOptIsSet("-s");
        AssertOptIsSet("-n");
        AssertOptIsSet("-c");
        AssertOptIsSet("-out");
        AssertOptIsSet("-i");


    }

public:
    void Usage(void) {
        std::cout << "Usage: ./dde -[param-id] [value] ... " << std::endl;
        std::cout << std::endl;
        std::cout << "-c [string]: path to configuration file" << std::endl;
        std::cout << "-s [int]: size of request" << std::endl;
        std::cout << "-n [int]: number of requests" << std::endl;
        std::cout << "-i [int]: storage index" << std::endl;
        std::cout << "Additional Parameters" << std::endl;
        std::cout << "-out [string]: path to csv file" << std::endl;
        std::cout << "" << std::endl;
    }

    BenchmarkArgs(int argc, char **argv) {
        AddOpt("-c", common::args::ArgType::kString);
        AddOpt("-out", common::args::ArgType::kString);
        AddOpt("-s", common::args::ArgType::kInt);
        AddOpt("-n", common::args::ArgType::kInt);
        AddOpt("-i", common::args::ArgType::kInt);
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
    if(rank == 0) SYMBIOS_CONF->ConfigureSymbiosServer();
    MPI_Barrier(MPI_COMM_WORLD);
    if(rank != 0) SYMBIOS_CONF->ConfigureSymbiosClient();
    SYMBIOS_CONF->DATA_DISTRIBUTION_POLICY = DataDistributionPolicy::RANDOM_POLICY;

    auto basket = BASKET_CONF;
    int request_size = args.GetIntOpt("-s");
    int number_request = args.GetIntOpt("-n");
    //auto distribution = DistributionFactory::Get(DistributionType::kUniform);
    //distribution->Shape((double)request_size/sizeof(double));

    std::shared_ptr<DataDistributionEngine> engine;
    if(rank == 0)
        engine = basket::Singleton<DataDistributionEngineFactory>::GetInstance()->GetDataDistributionEngine(SYMBIOS_CONF->DATA_DISTRIBUTION_POLICY);
    MPI_Barrier(MPI_COMM_WORLD);
    if(rank != 0)
        engine = basket::Singleton<DataDistributionEngineFactory>::GetInstance()->GetDataDistributionEngine(SYMBIOS_CONF->DATA_DISTRIBUTION_POLICY);
    MPI_Barrier(MPI_COMM_WORLD);
    auto request = Data();
    request.position_ = 0;
    request.storage_index_ = args.GetIntOpt("-i");;
    request.buffer_= static_cast<char *>(malloc(request_size));
    request.data_size_=request_size;
    ops_per_proc = number_request;
    bytes_per_proc = ops_per_proc * request_size;
    auto distributions = engine->Distribute(request);

    auto mo = basket::Singleton<MetadataOrchestrator>::GetInstance();
    auto path = SYMBIOS_CONF->STORAGE_SOLUTIONS[0]->end_point_;
    for(int i=0;i<number_request;++i){
        request.id_ = path + "/temp_" + std::to_string(i);
        mo->Delete(request);
    }

    common::debug::Timer store_t;
    for(int i=0;i<number_request;++i){
        request.id_ = path + "/temp_" + std::to_string(i);
        store_t.resumeTime();
        mo->Store(request,distributions);
        store_t.pauseTime();
    }
    printf("%f\n",store_t.getTimeElapsed());
    common::debug::Timer update_t;
    for(int i=0;i<number_request;++i){
        request.id_ = path + "/temp_" + std::to_string(i);
        update_t.resumeTime();
        mo->Store(request,distributions);
        update_t.pauseTime();
    }
    printf("%f\n",store_t.getTimeElapsed());
    Metadata primary_metadata;
    common::debug::Timer locate_t;
    for(int i=0;i<number_request;++i){
        request.id_ = path + "/temp_" + std::to_string(i);
        locate_t.resumeTime();
        mo->Locate(request,primary_metadata);
        locate_t.pauseTime();
        mo->Delete(request);
    }
    printf("%f\n",store_t.getTimeElapsed());

    MPI_Barrier(MPI_COMM_WORLD);
    double store_local_end_time = store_t.getTimeElapsed();
    double update_local_end_time = update_t.getTimeElapsed();
    double locate_local_end_time = locate_t.getTimeElapsed();
    double store_local_std_msec,update_local_std_msec,locate_local_std_msec;

    //Get global statistics
    double store_avg_msec, store_std_msec, store_min_msec, store_max_msec;
    double update_avg_msec, update_std_msec, update_min_msec, update_max_msec;
    double locate_avg_msec, locate_std_msec, locate_min_msec, locate_max_msec;

    MPI_Allreduce(&store_local_end_time, &store_avg_msec, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    MPI_Allreduce(&update_local_end_time, &update_avg_msec, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    MPI_Allreduce(&locate_local_end_time, &locate_avg_msec, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

    store_local_std_msec = pow(store_local_end_time - store_avg_msec, 2);
    update_local_std_msec = pow(update_local_end_time - update_avg_msec, 2);
    locate_local_std_msec = pow(locate_local_end_time - locate_avg_msec, 2);

    MPI_Reduce(&store_local_std_msec, &store_std_msec, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&store_local_std_msec, &store_min_msec, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&store_local_std_msec, &store_max_msec, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    MPI_Reduce(&update_local_std_msec, &update_std_msec, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&update_local_std_msec, &update_min_msec, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&update_local_std_msec, &update_max_msec, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    MPI_Reduce(&locate_local_std_msec, &locate_std_msec, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&locate_local_std_msec, &locate_min_msec, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&locate_local_std_msec, &locate_max_msec, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);


    store_avg_msec = store_avg_msec/nprocs;
    store_std_msec = std::sqrt(store_std_msec);

    update_avg_msec = update_avg_msec/nprocs;
    update_std_msec = std::sqrt(update_std_msec);

    locate_avg_msec = locate_avg_msec/nprocs;
    locate_std_msec = std::sqrt(locate_std_msec);


    //Get average bandwidth and throughput
    size_t tot_ops = ((size_t)ops_per_proc) * nprocs;
    size_t tot_bytes = bytes_per_proc * nprocs;
    double store_thrpt_kiops = ((double)tot_ops)/store_avg_msec;
    double store_bw_kbps = ((double)tot_bytes)/store_avg_msec;

    double update_thrpt_kiops = ((double)tot_ops)/update_avg_msec;
    double update_bw_kbps = ((double)tot_bytes)/update_avg_msec;


    double locate_thrpt_kiops = ((double)tot_ops)/locate_avg_msec;
    double locate_bw_kbps = ((double)tot_bytes)/locate_avg_msec;

    //Write to output CSV in root process
    if(rank == 0 && args.OptIsSet("-out")) {
        std::string output_path = args.GetStringOpt("-out");
        bool exists = boost::filesystem::exists(output_path);
        std::ofstream out(output_path, std::ofstream::out | std::ofstream::app);
        if(!exists) {
            out << "store_avg_msec,store_std_msec,store_min_msec,store_max_msec,"
                   "update_avg_msec,update_std_msec,update_min_msec,update_max_msec,"
                   "locate_avg_msec,locate_std_msec,locate_min_msec,locate_max_msec,"
                   "nprocs,number_request,request_size,seed,tot_ops,tot_bytes,"
                   "store_thrpt_kiops,store_bw_kbps,dde"
                   "update_thrpt_kiops,update_bw_kbps,dde"
                   "locate_thrpt_kiops,locate_bw_kbps,dde" << std::endl;
        }
        out <<
            store_avg_msec << "," << store_std_msec << "," << store_min_msec << "," << store_max_msec << "," <<
            update_avg_msec << "," << update_std_msec << "," << update_min_msec << "," << update_max_msec << "," <<
            locate_avg_msec << "," << locate_std_msec << "," << locate_min_msec << "," << locate_max_msec << "," <<
            nprocs << "," <<
            number_request << "," <<
            request_size << "," <<
            SYMBIOS_CONF->RANDOM_SEED << "," <<
            tot_ops << "," <<
            tot_bytes << "," <<
            store_thrpt_kiops << "," << store_bw_kbps << "," <<
            update_thrpt_kiops << "," << update_bw_kbps << "," <<
            locate_thrpt_kiops << "," << locate_bw_kbps << "," <<
            std::endl;
        std::cout << store_avg_msec << "," << store_std_msec << "," << store_min_msec << "," << store_max_msec << "," <<
                  update_avg_msec << "," << update_std_msec << "," << update_min_msec << "," << update_max_msec << "," <<
                  locate_avg_msec << "," << locate_std_msec << "," << locate_min_msec << "," << locate_max_msec << "," <<
                  nprocs << "," <<
                  number_request << "," <<
                  request_size << "," <<
                  SYMBIOS_CONF->RANDOM_SEED << "," <<
                  tot_ops << "," <<
                  tot_bytes << "," <<
                  store_thrpt_kiops << "," << store_bw_kbps << "," <<
                  update_thrpt_kiops << "," << update_bw_kbps << "," <<
                  locate_thrpt_kiops << "," << locate_bw_kbps << "," <<
                  std::endl;
    }

    MPI_Finalize();


}
