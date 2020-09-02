//
// Created by mani on 8/24/2020.
//

#include <mpi.h>
#include <common/arguments.h>
#include <symbios/storage_cost_predictor/storage_cost_predictor.h>
#include <basket/common/singleton.h>

class SCPArgs : public common::args::ArgMap {
private:
    void VerifyArgs(void) {
        AssertOptIsSet("-model");
        AssertOptIsSet("-num_reqs");
    }

public:
    void Usage(void) {
        std::cout << "Usage: ./test -[param-id] [value] ... " << std::endl;
        std::cout << std::endl;
        std::cout << "-model [string]: path to model file" << std::endl;
        std::cout << "-num_req [size]: The number of requests to run" << std::endl;
        std::cout << "" << std::endl;
    }

    SCPArgs(int argc, char **argv) {
        AddOpt("-model", common::args::ArgType::kString);
        AddOpt("-num_reqs", common::args::ArgType::kSize);
        ArgIter(argc, argv);
        VerifyArgs();
    }
};

void time_stats(double local_time_spent, int nprocs, double &avg_msec, double &std_msec, double &min_msec, double &max_msec)
{
    double local_std_msec;
    MPI_Allreduce(&local_time_spent, &avg_msec, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    avg_msec = avg_msec/nprocs;
    local_std_msec = pow(local_time_spent - avg_msec, 2);
    MPI_Reduce(&local_std_msec, &std_msec, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_time_spent, &min_msec, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_time_spent, &max_msec, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    std_msec = std::sqrt(std_msec);
}

void out_csv(SCPArgs &args, int rank, double local_time_spent, int nprocs, size_t nreqs_per_proc)
{
    if(!args.OptIsSet("-out")) {
        return;
    }

    //Get average time spent in application
    double avg_msec, std_msec, min_msec, max_msec;
    time_stats(local_time_spent, nprocs, avg_msec, std_msec, min_msec, max_msec);

    //Get average bandwidth and throughput
    size_t tot_ops = nreqs_per_proc * nprocs;
    double thrpt_kiops = ((double)tot_ops)/avg_msec;

    //Write to output CSV in root process
    if(rank == 0) {
        std::string conf = args.GetStringOpt("-config");
        std::string output_path = args.GetStringOpt("-out");
        bool exists = boost::filesystem::exists(output_path);
        std::ofstream out(output_path, std::ofstream::out | std::ofstream::app);
        if(!exists) {
            out << "avg_msec,std_msec,min_msec,max_msec,nprocs,md_fcnt,md_dir,tot_ops,thrpt_kiops,conf" << std::endl;
        }
        out <<
            avg_msec << "," <<
            std_msec << "," <<
            min_msec << "," <<
            max_msec << "," <<
            nprocs << "," <<
            tot_ops << "," <<
            thrpt_kiops << "," <<
            conf << std::endl;
    }
}

int main(int argc, char * argv[]) {
    MPI_Init(&argc,&argv);
    int rank=0, nprocs = 1;

    SCPArgs args(argc, argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    auto scp = basket::Singleton<StorageCostPredictor>::GetInstance();
    std::string config = "rank" + std::to_string(rank);
    size_t num_reqs = args.GetSizeOpt("-num_reqs");
    size_t nreqs_per_proc = num_reqs / nprocs;
    std::string model_path = args.GetStringOpt("-model");

    //Generate test data
    if(rank == 0) {
        std::ofstream of(model_path, std::ofstream::out | std::ofstream::trunc);
        for(size_t i = 0; i < nprocs; ++i) {
            std::string config = "rank" + std::to_string(i);
            of <<
               0 << "," <<
               .5 << "," <<
               .5 << "," <<
               config <<
               std::endl;
        }
    }

    //Run tests
    common::debug::Timer t;
    t.startTime();
    scp->Init(rank, nprocs, model_path);
    for(size_t i =0; i < nreqs_per_proc; ++i) {
        scp->Feedback(10*(rank+1)*(i+1), 25*(rank+1)*(i+1), 25*(rank+1)*(i+1), config);
        scp->Predict(25*(rank+1)*(i+1), 25*(rank+1)*(i+1), config);
    }
    scp->Finalize();
    double local_time_spent = t.endTime();

    //Get stats
    out_csv(args, rank, local_time_spent, nprocs, nreqs_per_proc);

    MPI_Finalize();
}