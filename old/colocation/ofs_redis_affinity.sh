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
xsize=$1
delay=$2
aff=$3

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

if [[ ${aff} == "numa" ]]
then
  cd ${OFS_SCRIPT_DIR}
  ./start-server-numa.sh
  cd ${REDIS_SCRIPT_DIR}
  ./start-numa.sh
elif [[ ${aff} == "interleave" ]]
then
  cd ${OFS_SCRIPT_DIR}
  ./start-server-interleave.sh
  cd ${REDIS_SCRIPT_DIR}
  ./start-interleave.sh
else
  cd ${OFS_SCRIPT_DIR}
  ./start-server.sh
  cd ${REDIS_SCRIPT_DIR}
  ./start.sh
fi


cd ${CWD}
first_client=`head -1 ${CLIENT_HOSTS} | cut -d':' -f1`
for ppn in ${PPNS[@]}
do
  for mode in ${MODES[@]}
  do
    echo ""
    echo "********************************************************"
    echo "Testing ofs_redis ${mode} access, with PPN=${ppn}, xsize=${xsize} ..."
    echo "********************************************************"
    trace_file=${CWD}/Redis/${mode}.csv
    for rep in `seq 1 ${REP}`
    do
      ssh ${first_client} "rm -rf /tmp/_redis_stop_flag_file"
      ${CWD}/redis.sh ${trace_file} ${ppn} ${delay} ${REDIS_TIME_LIMIT} >> ${REDIS_LOG} 2>&1 &
      ${CWD}/ior.sh ${xsize} ${mode} ${ppn} 0 >> ${IOR_LOG} 2>&1
      ssh ${first_client} "touch /tmp/_redis_stop_flag_file"

      sleep 1
      pgrep -x redis.sh | xargs kill -2
      pgrep -x redis.sh | xargs kill -9
      mpssh -f ${CWD}/nodes "killall -9 Redis"
      sleep ${PLOT_SEP_SLEEP}
      mpssh -f ${CWD}/nodes "sudo fm" > /dev/null 2>&1
      sleep ${PLOT_SEP_SLEEP}
    done # end of rep
  done # end of mode
  ${REDIS_SCRIPT_DIR}/flushall.sh > /dev/null 2>&1
done # end of ppn
