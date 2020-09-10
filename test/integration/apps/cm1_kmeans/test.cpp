//
// Created by mani on 8/24/2020.
//

#include <common/arguments.h>
#include <common/replayer.h>
#include <boost/filesystem.hpp>
#include <fstream>

class ReplayArgs : public common::args::ArgMap {
  private:
    void VerifyArgs(void) {
        AssertOptIsSet("-r");
        AssertOptIsSet("-m");
        AssertOptIsSet("-s");
        AssertOptIsSet("-t");
        AssertOptIsSet("-o");
        AssertOptIsSet("-c");
        AssertOptIsSet("-f");
    }

  public:
    void Usage(void) {
        std::cout << "Usage: ./cm1_kmeans -[param-id] [value] ... " << std::endl;
        std::cout << std::endl;
        std::cout << "-r [int]: number of repetitions" << std::endl;
        std::cout << "-c [int]: Chunk size of storage" << std::endl;
        std::cout << "-m [string]: Trace replay mode" << std::endl;
        std::cout << "  POSIX" << std::endl;
        std::cout << "  IRIS" << std::endl;
        std::cout << "  NIOBE" << std::endl;
        std::cout << "  SYMBIOS" << std::endl;
        std::cout << "-s [string]: Storage (I/O) type" << std::endl;
        std::cout << "  FILE" << std::endl;
        std::cout << "  MONGO" << std::endl;
        std::cout << "  REDIS" << std::endl;
        std::cout << "-t [string]: Trace directory" << std::endl;
        std::cout << "-o [string]: Output directory" << std::endl;
        std::cout << "-out [string]: output csv" << std::endl;
        std::cout << "-f [string]: Symbios configuration file (optional)" << std::endl;
    }

    ReplayArgs(int argc, char **argv) {
        AddOpt("-m", common::args::ArgType::kStringMap);
        AddStringMapVal("-m", "POSIX", 0);
        AddStringMapVal("-m", "IRIS", 1);
        AddStringMapVal("-m", "NIOBE", 2);
        AddStringMapVal("-m", "SYMBIOS", 3);
        AddOpt("-s", common::args::ArgType::kStringMap);
        AddStringMapVal("-s", "FILE", IOClientType::FILE_IO);
        AddStringMapVal("-s", "MONGO", IOClientType::MONGO_IO);
        AddStringMapVal("-s", "REDIS", IOClientType::REDIS_IO);
        AddOpt("-t", common::args::ArgType::kString);
        AddOpt("-o", common::args::ArgType::kString);
        AddOpt("-f", common::args::ArgType::kString);
        AddOpt("-out", common::args::ArgType::kString);
        AddOpt("-r", common::args::ArgType::kInt);
        AddOpt("-c", common::args::ArgType::kInt);
        ArgIter(argc, argv);
        VerifyArgs();
    }
};

int main(int argc, char * argv[]){
    MPI_Init(&argc, &argv);
    ReplayArgs args(argc, argv);
    int reps = args.GetIntOpt("-r");
    int mode = args.GetIntOpt("-m");
    int stor_type = args.GetIntOpt("-s");
    int chunk = args.GetIntOpt("-c");
    std::string trace_path = args.GetStringOpt("-t");
    std::string output_path = args.GetStringOpt("-o");
    std::string symbios_conf = args.GetStringOpt("-f");
    int my_rank,comm_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    trace_replayer tr;
    auto filename = output_path + boost::filesystem::path::preferred_separator + "data_" +std::to_string(my_rank)+".bat";
    double cm1_time = tr.replay_trace(trace_path + boost::filesystem::path::preferred_separator + "CM1.csv",filename , reps, my_rank, (IOLib)mode, stor_type, chunk, symbios_conf);
    double kmeans_time = tr.replay_trace(trace_path + boost::filesystem::path::preferred_separator + "Kmeans.csv", filename, reps, my_rank, (IOLib)mode, stor_type, chunk, symbios_conf);
    tr.clean_data(trace_path + boost::filesystem::path::preferred_separator + "CM1.csv", filename, reps, my_rank, (IOLib)mode, stor_type, chunk, symbios_conf);
    if(my_rank == 0 && args.OptIsSet("-out")) {
        std::string output_path = args.GetStringOpt("-out");
        bool exists = boost::filesystem::exists(output_path);

        std::stringstream stream;
        std::ofstream outfile(output_path, std::ofstream::out | std::ofstream::app);
        if(!exists) {
            stream << "cm1,kmeans,mode,stor_type,chunk,nprocs" << std::endl;
        }
        stream << cm1_time << "," << kmeans_time << "," << mode << "," << stor_type << "," << chunk << "," << comm_size << std::endl;
        outfile << stream.str();
        std::cout << stream.str();
    }
    MPI_Finalize();
    return 0;
}
