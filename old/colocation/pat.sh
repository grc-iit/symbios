#!/bin/bash

CWD="/home/kfeng/SuperServer/colocation"
PAT_DIR="/home/kfeng/pkg_src/PAT"
PAT_COLL_DIR="${PAT_DIR}/PAT-collecting-data"
PAT_DATA_DIR="${PAT_COLL_DIR}/results"
REDIS_LOG=${CWD}/redis.log
IOR_LOG=${CWD}/ior.log
DELAYS=(0 20 40 60 80)
XSIZES=(32k 128k 512k 2048k 8192k 16384k)
AFFINITY=("numa" "interleave")

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

cd ${PAT_COLL_DIR}
#################################################################################
# OrangeFS only
#################################################################################
#for xsize in ${XSIZES[@]}
#do
  #sed -i "s/colocate.sh.*/colocate.sh ofs_only ${xsize}/" ${PAT_COLL_DIR}/config
  #./pat run
  #${CWD}/analysis.sh ofs_only_${xsize}
#done

#################################################################################
# OrangeFS with idle Redis service process
#################################################################################
#for xsize in ${XSIZES[@]}
#do
  #sed -i "s/colocate.sh.*/colocate.sh ofs_idle_redis ${xsize}/" ${PAT_COLL_DIR}/config
  #./pat run
  #${CWD}/analysis.sh ofs_idle_redis_${xsize}
#done

#################################################################################
# Mixed workload: OrangeFS with x second delayed Redis
#################################################################################
#for xsize in ${XSIZES[@]}
#do
  #for delay in ${DELAYS[@]}
  #do
    #sed -i "s/colocate.sh.*/colocate.sh ofs_redis ${xsize} ${delay}/" ${PAT_COLL_DIR}/config
    #./pat run
    #${CWD}/analysis.sh ofs_redis_${xsize}_${delay}
  #done
#done

#################################################################################
# Mixed workloadi with affinity: OrangeFS with x second delayed Redis
#################################################################################
#for xsize in ${XSIZES[@]}
#do
  #for delay in ${DELAYS[@]}
  #do
    #for aff in ${AFFINITY[@]}
    #do
      #sed -i "s/colocate.sh.*/colocate.sh ofs_redis_affinity ${xsize} ${delay} ${aff}/" ${PAT_COLL_DIR}/config
      #./pat run
      #${CWD}/analysis.sh ofs_redis_affinity_${xsize}_${delay}_${aff}
    #done
  #done
#done

#################################################################################
# Redis with idle OrangeFS service process
#################################################################################
sed -i "s/colocate.sh.*/colocate.sh redis_idle_ofs/" ${PAT_COLL_DIR}/config
./pat run
${CWD}/analysis.sh redis_idle_ofs

#################################################################################
# Redis only
#################################################################################
sed -i "s/colocate.sh.*/colocate.sh redis_only/" ${PAT_COLL_DIR}/config
./pat run
${CWD}/analysis.sh redis_only

cd ${CWD}
