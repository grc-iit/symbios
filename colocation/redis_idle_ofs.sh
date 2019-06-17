#!/bin/bash

CWD="/home/kfeng/SuperServer/colocation"
SCRIPT_DIR="/home/kfeng/pkg_src/Utility-scripts"
OFS_SCRIPT_DIR="/home/kfeng/pkg_src/Utility-scripts/OrangeFS"
REDIS_SCRIPT_DIR="/home/kfeng/pkg_src/Utility-scripts/Redis"
PAT_DIR="/home/kfeng/pkg_src/PAT"
CLIENT_HOSTS=${CWD}/clients
REDIS_LOG=${CWD}/redis.log
IOR_LOG=${CWD}/ior.log
MODES=("write" "read")
PPNS=(40)
REP=1
PLOT_SEP_SLEEP=10
REDIS_TIME_LIMIT=60
delay=0

reset()
{
  cd ${OFS_SCRIPT_DIR}
  ./stop-client.sh
  ./stop-server.sh
  ./clean.sh
  cd ${REDIS_SCRIPT_DIR}
  ./stop.sh
  ./clean.sh
  cd ${CWD}
}

reset

cd ${REDIS_SCRIPT_DIR}
./start.sh
cd ${OFS_SCRIPT_DIR}
./start-server.sh

cd ${CWD}
for ppn in ${PPNS[@]}
do
  for mode in ${MODES[@]}
  do
    echo ""
    echo "********************************************************"
    echo "Testing redis_idle_ofs ${mode} access, with PPN=${ppn} ..."
    echo "********************************************************"
    trace_file=${CWD}/Redis/${mode}.csv
    for rep in `seq 1 ${REP}`
    do
      ${CWD}/redis.sh ${trace_file} ${ppn} ${delay} ${REDIS_TIME_LIMIT} >> ${REDIS_LOG} 2>&1

      sleep ${PLOT_SEP_SLEEP}
      mpssh -f ${CWD}/nodes "sudo fm" > /dev/null 2>&1
      sleep ${PLOT_SEP_SLEEP}
    done # end of rep
  done # end of mode
  ${REDIS_SCRIPT_DIR}/flushall.sh > /dev/null 2>&1
done # end of ppn
