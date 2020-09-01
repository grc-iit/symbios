//
// Created by mani on 8/24/2020.
//

#include <common/arguments.h>
#include <src/symbios/storage_cost_predictor/storage_cost_predictor.h>

class SCCArgs : public common::args::ArgMap {
private:
    void VerifyArgs(void) {
    }

public:
    void Usage(void) {
        std::cout << "Usage: ./test -[param-id] [value] ... " << std::endl;
        std::cout << std::endl;
        std::cout << "-metrics [string]: path to metrics file" << std::endl;
        std::cout << "-model [string]: path to model file" << std::endl;
        std::cout << "" << std::endl;
    }

    SCCArgs(int argc, char **argv) {
        AddOpt("-metrics", common::args::ArgType::kString, "~/metrics.csv");
        AddOpt("-model", common::args::ArgType::kString, "~/model.csv");
        ArgIter(argc, argv);
        VerifyArgs();
    }
};

int main(int argc, char * argv[]) {
    MPI_Init(&argc,&argv);
    int rank, nprocs = 1;

    SCCArgs args(argc, argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    StorageCostPredictor a;
    std::string config = "asdf";
    a.Init();
    a.LoadMetrics(rank, nprocs, args.GetStringOpt("-metrics"), args.GetStringOpt("-model"));
    a.Feedback(10, 25, 25, config);
    a.Feedback(20, 50, 50, config);
    a.Feedback(30, 75, 75, config);
    a.Fit();
    std::cout << a.Predict(100.0, 100.0, config) << std::endl;
    return 0;
}