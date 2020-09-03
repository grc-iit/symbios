#!/bin/bash

#Test the storage cost predictor for various request sizes and process counts
#Will output results to ${CMAKE_BINARY_DIR}/scc_thrpt.csv

NPROCS=$1       #Number of MPI processes to create
DATA_DIR=$2     #Where you want to store model data/information
NREQS=$3        #The number of requests per process
CMAKE_BINARY_DIR=$4   #The path to the CMAKE build directory (by default, the current working directory)
if [ $# -lt 4 ]; then
  CMAKE_BINARY_DIR=$(pwd)
fi

mpirun -n ${NPROCS} ${CMAKE_BINARY_DIR}/test/unit/unit_storage_cost_predictor -model ${CMAKE_BINARY_DIR}/model.csv -nreqs ${NREQS} -out ${CMAKE_BINARY_DIR}/scc_thrpt.csv

#Optionally, you can also use ctest to run tests, but this requires you to be cd'd into CMAKE_BINARY_DIR
#For all tests: ctest --verbose -R sc_predictor*
#For specific tests: ctest --verbose -R sc_predictor_${NREQS}_${NPROCS}