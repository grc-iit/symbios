//
// Created by mani on 8/24/2020.
//
#include <common/replayer.h>
int main(int argc, char * argv[]){
    MPI_Init(&argc, &argv);

    int reps = 1; //argv[0];

    trace_replayer tr;
    tr.prepare_data("WRFA.csv");
    tr.replay_trace("WRFA.csv", "wrfa_out.csv", reps, 0);
    tr.prepare_data("WRFC.csv");
    tr.replay_trace("WRFC.csv", "wrfc_out.csv", reps, 0);

    MPI_Finalize();
    return 0;
}
