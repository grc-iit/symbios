#!/bin/bash

REDIS_SCRIPT_DIR=/home/kfeng/pkg_src/Utility-scripts/Redis
CWD=/home/kfeng/SuperServer/single_hop/redis
REP=10

_self="${0##*/}"
fname=${_self%.sh}
log_file=${fname}.log

rm -f ${log_file}
for rep in `seq 1 ${REP}`
do
  cd ${REDIS_SCRIPT_DIR}
  ./flushall.sh
  cd ${CWD}
  ./redis_create_shadow testfile >> ${log_file}
done
