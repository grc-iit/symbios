//
// Created by mani on 8/24/2020.
//

#include <common/arguments.h>
#include <common/replayer.h>
#include <boost/filesystem.hpp>

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
        std::cout << "Usage: ./wrf -[param-id] [value] ... " << std::endl;
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
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    trace_replayer tr;
    tr.prepare_data(trace_path + boost::filesystem::path::preferred_separator + "WRFC.csv", output_path + boost::filesystem::path::preferred_separator + "wrfc_out.csv", reps, my_rank, (IOLib)mode, stor_type, chunk, symbios_conf);
    tr.replay_trace(trace_path + boost::filesystem::path::preferred_separator + "WRFC.csv", output_path + boost::filesystem::path::preferred_separator + "wrfc_out.csv", reps, my_rank, (IOLib)mode, stor_type, chunk, symbios_conf);
    tr.prepare_data(trace_path + boost::filesystem::path::preferred_separator + "WRFA.csv", output_path + boost::filesystem::path::preferred_separator + "wrfa_out.csv", reps, my_rank, (IOLib)mode, stor_type, chunk, symbios_conf);
    tr.replay_trace(trace_path + boost::filesystem::path::preferred_separator + "WRFA.csv", output_path + boost::filesystem::path::preferred_separator + "wrfa_out.csv", reps, my_rank, (IOLib)mode, stor_type, chunk, symbios_conf);

    MPI_Finalize();
    return 0;
}
