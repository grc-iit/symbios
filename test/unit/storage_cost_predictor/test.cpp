//
// Created by mani on 8/24/2020.
//

#include <common/arguments.h>
#include <src/symbios/storage_cost_predictor/storage_cost_predictor.h>
#include <basket/common/singleton.h>

class SCCArgs : public common::args::ArgMap {
private:
    void VerifyArgs(void) {
        AssertOptIsSet("-metrics");
        AssertOptIsSet("-model");
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
        AddOpt("-metrics", common::args::ArgType::kString);
        AddOpt("-model", common::args::ArgType::kString);
        ArgIter(argc, argv);
        VerifyArgs();
    }
};

int main(int argc, char * argv[]) {
    MPI_Init(&argc,&argv);
    int rank=0, nprocs = 1;

    SCCArgs args(argc, argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    std::shared_ptr<StorageCostPredictor> a = basket::Singleton<StorageCostPredictor>::GetInstance();
    std::string config = "asdf";
    a->LoadMetrics(rank, nprocs, args.GetStringOpt("-metrics"), args.GetStringOpt("-model"));
    a->Feedback(10*(rank+1), 25*(rank+1), 25*(rank+1), config);
    a->Feedback(20*(rank+1), 50*(rank+1), 50*(rank+1), config);
    a->Feedback(30*(rank+1), 75*(rank+1), 75*(rank+1), config);
    a->FitCommit();
    std::cout << a->Predict(100.0, 100.0, config) << std::endl;
    //MPI_Finalize();
}