#!/bin/bash

exec=/tmp/tmp.BUKlhPiLxF/build/test/integration/dde
conf=/tmp/tmp.BUKlhPiLxF/conf/symbios.conf
csv=/home/hdevarajan/dde.csv
rm $csv

declare -a policies=("RANDOM_POLICY" "ROUND_ROBIN_POLICY") # "RANDOM_POLICY" "ROUND_ROBIN_POLICY" "HEURISTICS_POLICY" "DYNAMIC_PROGRAMMING_POLICY")
declare -a request_sizes=(8192 16384 32768 65536 131072 262144 524288 1048576 2097152 4194304 8388608 16777216)
declare -a request_numbers=( 100 1000 10000 100000)
declare -a num_procs=(8)  #1 2 4 8 16 32 40)

for policy in "${policies[@]}"
do
  for request_size in "${request_sizes[@]}"
  do
    for request_number in "${request_numbers[@]}"
    do
      for num_proc in "${num_procs[@]}"
      do
        echo "mpirun -n $num_proc $exec -p $policy -c $conf -out $csv -s $request_size -n $request_number -seed 100"
        mpirun -n $num_proc $exec -p $policy -c $conf -out $csv -s $request_size -n $request_number -seed 100
        echo "waiting 5 seconds"
        sleep 5
      done
    done
  done
done