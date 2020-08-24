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
xsize=$1

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

cd ${OFS_SCRIPT_DIR}
./start-server.sh
cd ${REDIS_SCRIPT_DIR}
./start.sh

cd ${CWD}
for ppn in ${PPNS[@]}
do
  for mode in ${MODES[@]}
  do
    echo ""
    echo "********************************************************"
    echo "Testing ofs_idle_redis ${mode} access, with PPN=${ppn}. xsize=${xsize} ..."
    echo "********************************************************"
    for rep in `seq 1 ${REP}`
    do
      ${CWD}/ior.sh ${xsize} ${mode} ${ppn} 0 >> ${IOR_LOG} 2>&1
      sleep ${PLOT_SEP_SLEEP}
      mpssh -f ${CWD}/nodes "sudo fm" > /dev/null 2>&1
      sleep ${PLOT_SEP_SLEEP}
    done # end of rep
  done # end of mode
done # end of ppn
