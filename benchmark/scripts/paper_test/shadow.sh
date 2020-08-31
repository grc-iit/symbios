#!/bin/bash

exec=/tmp/tmp.BUKlhPiLxF/build/test/integration/shadow_link
conf=/tmp/tmp.BUKlhPiLxF/conf/symbios.conf
csv=/home/hdevarajan/shadow.csv
#fix this manually within configuration manager.
pfs_meta=/home/hdevarajan/pfs/meta

rm $csv
mkdir -p ${pfs_meta}
for i in ${pfs_meta}/*; do rm "$i"; done
mkdir -p ${pfs_meta}


declare -a request_sizes=(8192 16384 32768 65536 131072 262144 524288 1048576 2097152 4194304 8388608 16777216)
declare -a request_numbers=(100 1000 10000 100000)
declare -a num_procs=(1)  #1 2 4 8 16 32 40)

for request_size in "${request_sizes[@]}"
do
  for request_number in "${request_numbers[@]}"
  do
    for num_proc in "${num_procs[@]}"
    do
      echo "mpirun -n $num_proc $exec -c $conf -out $csv -s $request_size -n $request_number"
      mpirun -n $num_proc $exec -c $conf -out $csv -s $request_size -n $request_number
      echo "waiting 5 seconds"
      sleep 5
      for i in ${pfs_meta}/*; do rm "$i"; done
    done
  done
done

for i in ${pfs_meta}/*; do rm "$i"; done