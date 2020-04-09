#!/bin/bash

CWD="/home/kfeng/SuperServer/internal_component/colocation"
SCRIPT_DIR="/home/kfeng/pkg_src/Utility-scripts"
OFS1_SCRIPT_DIR="/home/kfeng/pkg_src/Utility-scripts/OrangeFS"
OFS2_SCRIPT_DIR="/home/kfeng/pkg_src/Utility-scripts/OrangeFS2"
OUTPUT_LOG=${CWD}/colocation.log
OUTPUT_LOG_IOR1=${CWD}/ior1.log
OUTPUT_LOG_IOR2=${CWD}/ior2.log

N_SERVER_TOTAL=24
N_CLIENT_TOTAL=24
N_SERVER_LIST_OFS1=(24 0 24 24 24 24 8 16)
N_SERVER_LIST_OFS2=(0 24 4 8 16 24 24 24)
#N_SERVER_LIST_OFS1=(24 0 24)
#N_SERVER_LIST_OFS2=(4 24 0)
XSIZE1_IN_KB=128
XSIZE2_IN_MB=4
BSIZE_IN_MB=32
REP=3
IOR_CLIENT_HOSTS=${CWD}/clients
IOR_ALL_HOSTS=${CWD}/nodes
ALL_COMP_HOSTS=${CWD}/all_comp_nodes
ALL_STOR_HOSTS=${CWD}/all_stor_nodes

OFS1_STRIPE_SIZE=4096
OFS2_STRIPE_SIZE=131072

IFACE=enp47s0
IOR_BIN=~/pkg_src/ior/src/ior.mpich
DEST_FILE1=pvfs2://mnt/nvme/kfeng/pvfs2-mount/ior.test
DEST_FILE2=pvfs2://mnt/nvme/kfeng/pvfs2-mount2/ior.test
FM=fm

cleanup()
{
  cd ${OFS1_SCRIPT_DIR}
  ./clean.sh > /dev/null
  cd ${OFS2_SCRIPT_DIR}
  ./clean.sh > /dev/null
  cd ${CWD}
}

gen_ofs_host_file()
{
  n_server_ofs1=$1
  n_server_ofs2=$2

  if [[ ${n_server_ofs1} -ne 0 ]]
  then
    cd ${OFS1_SCRIPT_DIR}
    rm -rf pvfs2*.conf
    cat ${ALL_STOR_HOSTS} | head -n ${n_server_ofs1} > servers
    sed -i "s/STRIPE_SIZE=.*/STRIPE_SIZE=${OFS1_STRIPE_SIZE}/" env.sh
    ./genconfig.sh
  fi

  if [[ ${n_server_ofs2} -ne 0 ]]
  then
    cd ${OFS2_SCRIPT_DIR}
    rm -rf pvfs2*.conf
    cat ${ALL_STOR_HOSTS} | head -n ${n_server_ofs2} > servers
    sed -i "s/STRIPE_SIZE=.*/STRIPE_SIZE=${OFS2_STRIPE_SIZE}/" env.sh
    ./genconfig.sh
  fi
  cd ${CWD}
}

start_ofs()
{
  n_server_ofs1=$1
  n_server_ofs2=$2

  if [[ ${n_server_ofs1} -ne 0 ]]
  then
    cd ${OFS1_SCRIPT_DIR}
    ./start-server.sh
  fi

  if [[ ${n_server_ofs2} -ne 0 ]]
  then
    cd ${OFS2_SCRIPT_DIR}
    ./start-server.sh
  fi
  cd ${CWD}
}

n_ofs1_conf=${#N_SERVER_LIST_OFS1[@]}
n_ofs2_conf=${#N_SERVER_LIST_OFS2[@]}
if [[ ${n_ofs1_conf} != ${n_ofs2_conf} ]]
then
  echo "Number of OFS1 and OFS2 configurations do not match, exiting ..."
  exit
fi

rm -rf ${OUTPUT_LOG}
rm -rf ${OUTPUT_LOG_IOR1}
rm -rf ${OUTPUT_LOG_IOR1}
cd ${CWD}
((n_conf=${#N_SERVER_LIST_OFS1[@]}-1))
for i in `seq 0 ${n_conf}`
do
  echo "Cleaning up ..."
  cleanup
  n_server_ofs1=${N_SERVER_LIST_OFS1[i]}
  n_server_ofs2=${N_SERVER_LIST_OFS2[i]}
  echo "Generating host files ..."
  gen_ofs_host_file ${n_server_ofs1} ${n_server_ofs2}
  echo "Starting OrangeFS ..."
  start_ofs ${n_server_ofs1} ${n_server_ofs2}
  for rep in `seq 1 ${REP}`
  do
    echo "" >> ${OUTPUT_LOG}
    echo "************************************************************" >> ${OUTPUT_LOG}
    echo "Co-locating OrangeFS (#server_ofs1=${n_server_ofs1}, #server_ofs2=${n_server_ofs2}), rep=${rep} ..." >> ${OUTPUT_LOG}
    diff ${OFS1_SCRIPT_DIR}/servers ${OFS2_SCRIPT_DIR}/servers >> ${OUTPUT_LOG}
    echo "************************************************************" >> ${OUTPUT_LOG}

    ((np=${N_CLIENT_TOTAL}*20))
    if [[ ${n_server_ofs1} -ne 0 ]]
    then
      echo "---------------------------------------------------------------------------------" >> ${OUTPUT_LOG}
      echo OFS1 command: /home/kfeng/MPICH/bin/mpiexec -n ${np} -iface ${IFACE} -f ${IOR_CLIENT_HOSTS} -prepend-rank -genv PVFS2TAB_FILE /mnt/nvme/kfeng/pvfs2tab ${IOR_BIN} -a MPIIO -t ${XSIZE1_IN_KB}k -b ${BSIZE_IN_MB}m -E -k -w -o ${DEST_FILE1} >> ${OUTPUT_LOG}
      echo "---------------------------------------------------------------------------------" >> ${OUTPUT_LOG}
      /home/kfeng/MPICH/bin/mpiexec -n ${N_CLIENT_TOTAL} -iface ${IFACE} -f ${IOR_CLIENT_HOSTS} -prepend-rank -genv PVFS2TAB_FILE /mnt/nvme/kfeng/pvfs2tab ${IOR_BIN} -a MPIIO -t ${XSIZE1_IN_KB}k -b ${BSIZE_IN_MB}m -E -k -w -o ${DEST_FILE1} >> ${OUTPUT_LOG_IOR1} &
    fi

    if [[ ${n_server_ofs2} -ne 0 ]]
    then
      echo "---------------------------------------------------------------------------------" >> ${OUTPUT_LOG}
      echo OFS2 command: /home/kfeng/MPICH/bin/mpiexec -n ${np} -iface ${IFACE} -f ${IOR_CLIENT_HOSTS} -prepend-rank -genv PVFS2TAB_FILE /mnt/nvme/kfeng/pvfs2tab ${IOR_BIN} -a MPIIO -t ${XSIZE2_IN_MB}m -b ${BSIZE_IN_MB}m -E -k -w -o ${DEST_FILE2} >> ${OUTPUT_LOG}
      echo "---------------------------------------------------------------------------------" >> ${OUTPUT_LOG}
      /home/kfeng/MPICH/bin/mpiexec -n ${N_CLIENT_TOTAL} -iface ${IFACE} -f ${IOR_CLIENT_HOSTS} -prepend-rank -genv PVFS2TAB_FILE /mnt/nvme/kfeng/pvfs2tab ${IOR_BIN} -a MPIIO -t ${XSIZE2_IN_MB}m -b ${BSIZE_IN_MB}m -E -k -w -o ${DEST_FILE2} >> ${OUTPUT_LOG_IOR2} &
    fi

    wait

    echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" >> ${OUTPUT_LOG}

    pvfs2_error=`tail -100 ${OUTPUT_LOG} | grep "\[E " | wc -l`
    if [[ ${pvfs2_error} != 0 ]]
    then
      echo "" >> ${OUTPUT_LOG}
      echo "" >> ${OUTPUT_LOG}
      echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" >> ${OUTPUT_LOG}
      echo "PVFS2 error found" >> ${OUTPUT_LOG}
      echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" >> ${OUTPUT_LOG}
      echo "" >> ${OUTPUT_LOG}
      echo "" >> ${OUTPUT_LOG}
    fi

    mpssh -f ${IOR_ALL_HOSTS} "sudo fm" > /dev/null 2>&1
    echo ""
    echo "Co-locating OrangeFS (#server_ofs1=${n_server_ofs1}, #server_ofs2=${n_server_ofs2}), rep=${rep} finishes, sleep for 5 seconds..."
    echo ""
    sleep 5
  done
done
