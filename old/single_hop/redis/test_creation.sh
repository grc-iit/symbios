#!/bin/bash

CWD="$(cd "$(dirname "${BASH_SOURCE[0]}")" > /dev/null 2>&1 && pwd)"
REDIS_SCRIPT_DIR=/home/kfeng/pkg_src/Utility-scripts/Redis
CLIENT_HOST_FILE=${CWD}/clients
SERVER_HOST_FILE=${CWD}/servers
PPN=(10 20 40)
REP=10

n_client=$(cat ${CLIENT_HOST_FILE} | wc -l)
_self="${0##*/}"
fname=${_self%.sh}
log_file=${fname}.log

rm -f ${log_file}
for ppn in ${PPN[@]}
do
  ((np=${ppn}*${n_client}))
  for rep in `seq 1 ${REP}`
  do
    echo Run ${rep} ...
    cd ${REDIS_SCRIPT_DIR}
    ./flushall.sh > /dev/null
    cd ${CWD}
    echo mpiexec -n ${np} -f ${CLIENT_HOST_FILE} ./redis_create_shadow_mt testfile >> ${log_file}
    mpiexec -n ${np} -f ${CLIENT_HOST_FILE} ./redis_create_shadow_mt testfile >> ${log_file}
    mpssh -f ${SERVER_HOST_FILE} 'sudo fm' > /dev/null
  done
done
