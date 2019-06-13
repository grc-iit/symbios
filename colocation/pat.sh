#!/bin/bash

CWD="/home/kfeng/SuperServer/colocation"
PAT_DIR="/home/kfeng/pkg_src/PAT"
PAT_COLL_DIR="${PAT_DIR}/PAT-collecting-data"
PAT_DATA_DIR="${PAT_COLL_DIR}/results"
REDIS_LOG=${CWD}/redis.log
IOR_LOG=${CWD}/ior.log
DELAYS=(0 5 10 15 20 25 30 35 40)
XSIZES=(32k 128k 512k 2048k 8192k 16384k)

if [ -t 1 ] # check if running in a terminal
then
  read -p "Overwrite previous log? " -n 1 -r
  echo    # (optional) move to a new line
  if [[ $REPLY =~ ^[Yy]$ ]]
  then
    rm -rf ${REDIS_LOG}
    rm -rf ${IOR_LOG}
    rm -rf ${PAT_DATA_DIR}/ofs_only_*
    rm -rf ${PAT_DATA_DIR}/ofs_redis_*
  else
    timestamp=`date +"%Y-%m-%d-%H-%M-%S-%N"`
    mv ${REDIS_LOG} ${REDIS_LOG}.${timestamp}
    mv ${IOR_LOG} ${IOR_LOG}.${timestamp}
  fi
else # running in nohup
  echo "Not attached to terminal, removing previous logs ..."
  rm -rf ${REDIS_LOG}
  rm -rf ${IOR_LOG}
  rm -rf ${PAT_DATA_DIR}/ofs_only_*
  rm -rf ${PAT_DATA_DIR}/ofs_redis_*
fi

#################################################################################
# OrangeFS only
#################################################################################
cd ${PAT_COLL_DIR}
#for xsize in ${XSIZES[@]}
#do
  #sed -i "s/colocate.sh.*/colocate.sh ofs_only ${xsize}/" ${PAT_COLL_DIR}/config
  #./pat run
  #${CWD}/analysis.sh ofs_only_${xsize}
#done

#################################################################################
# Mixed workload: OrangeFS with x second delayed Redis
#################################################################################
for xsize in ${XSIZES[@]}
do
  for delay in ${DELAYS[@]}
  do
    sed -i "s/colocate.sh.*/colocate.sh ofs_redis ${xsize} ${delay}/" ${PAT_COLL_DIR}/config
    ./pat run
    ${CWD}/analysis.sh ofs_redis_${xsize}_${delay}
  done
done
cd ${CWD}
