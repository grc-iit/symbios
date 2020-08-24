#!/bin/bash

CWD="/home/kfeng/SuperServer/colocation"

if [[ $# != 4 ]]
then
  echo "Usage: ./redis.sh trace_file ppn delay time_limit"
  echo "Exiting ..."
  exit
fi

IFACE=enp47s0
SERVER_HOSTS=${CWD}/servers
CLIENT_HOSTS=${CWD}/clients
REDIS_RW_BIN=/mnt/common/kfeng/SuperServer/colocation/Redis/cmake-build-debug/Redis

nclient=`cat ${CLIENT_HOSTS} | wc -l`
trace_file=$1
ppn=$2
delay=$3
time_limit=$4
echo "Redis test of trace file ${trace_file} with a delay of ${delay} seconds"
echo "Redis starts: `date +"%Y-%m-%d-%H-%M-%S-%N"`"
trap 'echo "Redis ends: `date +'%Y-%m-%d-%H-%M-%S-%N'`"' KILL
trap 'echo "Redis ends: `date +'%Y-%m-%d-%H-%M-%S-%N'`"' INT

((np=${ppn}*${nclient}))
sleep ${delay}
echo /home/kfeng/MPICH/bin/mpiexec -n ${np} -iface ${IFACE} -f ${CLIENT_HOSTS} ${REDIS_RW_BIN} ${trace_file} ${time_limit} 0
/home/kfeng/MPICH/bin/mpiexec -n ${np} -iface ${IFACE} -f ${CLIENT_HOSTS} ${REDIS_RW_BIN} ${trace_file} ${time_limit} 0
