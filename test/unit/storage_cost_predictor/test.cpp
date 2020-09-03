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
        AssertOptIsSet("-nreqs");
        AssertOptIsSet("-out");
    }

public:
    void Usage(void) {
        std::cout << "Usage: ./test -[param-id] [value] ... " << std::endl;
        std::cout << std::endl;
        std::cout << "-model [string]: path to model file" << std::endl;
        std::cout << "-nreqs [size]: The number of requests to run" << std::endl;
        std::cout << "-out [string]: The request output file" << std::endl;
        std::cout << "" << std::endl;
    }

    SCPArgs(int argc, char **argv) {
        AddOpt("-model", common::args::ArgType::kString);
        AddOpt("-nreqs", common::args::ArgType::kSize);
        AddOpt("-out", common::args::ArgType::kString);
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

void out_csv(SCPArgs &args, int rank, common::debug::Timer t[4], int nprocs, size_t nreqs_per_proc)
{
    if(!args.OptIsSet("-out")) {
        return;
    }

    //Get time spent in application process
    double net_time[4];
    for(int i = 0; i < 4; ++i) {
        net_time[i] = t[i].getTimeElapsed();
    }

    //Get average time spent in application
    double avg_msec[4], std_msec[4], min_msec[4], max_msec[4];
    for(int i = 0; i < 4; ++i) {
        time_stats(net_time[i], nprocs, avg_msec[i], std_msec[i], min_msec[i], max_msec[i]);
    }
    std::string prefix[] = {"init_", "feed_", "pred_", "fin_"};

    //Get average bandwidth and throughput
    size_t tot_ops = nreqs_per_proc * nprocs;
    double thrpt_kiops[4] = {0}, sum = 0;
    double ops[] = {1, (double)tot_ops, (double)tot_ops, 1};
    for(int i = 0; i < 4; ++i) {
        thrpt_kiops[i] = ((double)ops[i])/avg_msec[i];
        sum += avg_msec[i];
    }
    double net_kiops = ((double)tot_ops)/sum;

    //Write to output CSV in root process
    if(rank == 0) {
        std::string output_path = args.GetStringOpt("-out");
        bool exists = boost::filesystem::exists(output_path);
        std::ofstream out(output_path, std::ofstream::out | std::ofstream::app);

        //Create CSV Header
        if(!exists) {
            std::string header;
            for (int i = 0; i < 4; ++i) {
                header += prefix[i] + "msec_avg" + "," +
                          prefix[i] + "msec_std" + "," +
                          prefix[i] + "min_std" + "," +
                          prefix[i] + "max_std" + "," +
                          prefix[i] + "thrpt_kiops" + ",";
            }
            header += "net_kiops,nprocs,nreqs_per_proc,nreqs";
            out << header << std::endl;
        }

        //Push observations to CSV
        for (int i = 0; i < 4; ++i) {
            out <<
                avg_msec[i] << "," <<
                std_msec[i] << "," <<
                min_msec[i] << "," <<
                max_msec[i] << "," <<
                thrpt_kiops[i] << ",";
        }
        out << net_kiops << "," << nprocs << "," << nreqs_per_proc << "," << tot_ops << std::endl;
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
    size_t nreqs_per_proc = args.GetSizeOpt("-nreqs");
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
    common::debug::Timer t[4];

    t[0].resumeTime();
    scp->Init(rank, nprocs, model_path, true);
    t[0].pauseTime();

    for(size_t i =0; i < nreqs_per_proc; ++i) {
        t[1].resumeTime();
        scp->Feedback(10*(rank+1)*(i+1), 25*(rank+1)*(i+1), -25*(rank+1)*(i+1), config);
        t[1].pauseTime();

        t[2].resumeTime();
        scp->Predict(25*(rank+1)*(i+1), -25*(rank+1)*(i+1), config);
        t[2].pauseTime();
    }

    t[3].resumeTime();
    scp->Finalize();
    t[3].pauseTime();

    //Get stats
    out_csv(args, rank, t, nprocs, nreqs_per_proc);
    MPI_Finalize();
    return 0;
}