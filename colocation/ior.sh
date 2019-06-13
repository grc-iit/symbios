#!/bin/bash

CWD="/home/kfeng/SuperServer/colocation"

if [[ $# != 4 ]]
then
  echo "Usage: ./ior.sh xsize read|write ppn delay"
  echo "Exiting ..."
  exit
fi

IFACE=enp47s0
SERVER_HOSTS=${CWD}/servers
CLIENT_HOSTS=${CWD}/clients
IOR_BIN=~/pkg_src/ior/src/ior
DEST_FILE=pvfs2://mnt/nvme/kfeng/pvfs2-mount/ior.test
FM=fm
COLL_IO=''
IO_MODE='-w'
REP=1
xsize=$1
mode=$2
ppn=$3
delay=$4

nserver=`cat ${SERVER_HOSTS} | wc -l`
nclient=`cat ${CLIENT_HOSTS} | wc -l`
bsize=32

COLL_IO=''
if [[ ${mode} == "read" ]]
then
  rw="-r"
elif [[ ${mode} == "write" ]]
then
  rw="-w"
fi

if [[ ${delay} != 0 ]]
then
  sleep ${delay}
fi

echo "IOR sequential independent ${mode} test"
echo "IOR starts: `date +"%Y-%m-%d-%H-%M-%S-%N"`"
((np=${ppn}*${nclient}))
/home/kfeng/MPICH/bin/mpiexec -n ${np} -iface ${IFACE} -f ${CLIENT_HOSTS} ${IOR_BIN} -a MPIIO -t ${xsize} -b ${bsize}m -H ${COLL_IO} -E -k ${rw} -o ${DEST_FILE}
echo "IOR ends: `date +'%Y-%m-%d-%H-%M-%S-%N'`"
