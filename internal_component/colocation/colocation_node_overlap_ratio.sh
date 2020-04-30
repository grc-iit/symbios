#!/bin/bash

CWD="/home/kfeng/SuperServer/internal_component/colocation"
SCRIPT_DIR="/home/kfeng/pkg_src/Utility-scripts"
OFS1_SCRIPT_DIR="/home/kfeng/pkg_src/Utility-scripts/OrangeFS"
OFS2_SCRIPT_DIR="/home/kfeng/pkg_src/Utility-scripts/OrangeFS2"
OUTPUT_LOG=${CWD}/colocation.log
OUTPUT_LOG_IOR1=${CWD}/output_ior1.log
OUTPUT_LOG_IOR2=${CWD}/output_ior2.log
OUTPUT_RES=${CWD}/result.log

N_SERVER_TOTAL=24
N_CLIENT_TOTAL=24
N_SERVER_LIST_OFS1=(24 0 24 24 24 24 24 24 20 16 12 8 4)
N_SERVER_LIST_OFS2=(0 24 4 8 12 16 20 24 24 24 24 24 24)
#N_SERVER_LIST_OFS1=(12)
#N_SERVER_LIST_OFS2=(12)
XSIZE1_IN_KB=128
XSIZE2_IN_MB=4
BSIZE1_IN_MB=8
BSIZE2_IN_MB=64
REP=10
IOR_CLIENT_HOSTS=${CWD}/clients
IOR_ALL_HOSTS=${CWD}/all_nodes
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
  rm -f servers
  cd ${OFS2_SCRIPT_DIR}
  ./clean.sh > /dev/null
  rm -f servers
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
    cat ${ALL_STOR_HOSTS} | head -n ${n_server_ofs1} > ${OFS1_SCRIPT_DIR}/servers
    sed -i "s/STRIPE_SIZE=.*/STRIPE_SIZE=${OFS1_STRIPE_SIZE}/" env.sh
    ./genconfig.sh
  fi

  if [[ ${n_server_ofs2} -ne 0 ]]
  then
    cd ${OFS2_SCRIPT_DIR}
    rm -rf pvfs2*.conf
    if [[ ${n_server_ofs1} -eq ${N_SERVER_TOTAL} || ${n_server_ofs2} -eq ${N_SERVER_TOTAL} ]]
    then
      cat ${ALL_STOR_HOSTS} | head -n ${n_server_ofs2} > ${OFS2_SCRIPT_DIR}/servers
    else
      ((n_server_in_use=${n_server_ofs1}+${n_server_ofs2}))
      if [[ ${n_server_in_use} == ${N_SERVER_TOTAL} ]]
      then
        cat ${ALL_STOR_HOSTS} | tail -n ${n_server_ofs2} > ${OFS2_SCRIPT_DIR}/servers
      fi
    fi
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

mv -f ${OUTPUT_LOG} ${OUTPUT_LOG}.bak
mv -f ${OUTPUT_RES} ${OUTPUT_RES}.bak
this_script=$(basename ${BASH_SOURCE[0]})
cp -f ${this_script} README.script_snapshot
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
    echo ""                                                                                                     >> ${OUTPUT_LOG}
    echo "************************************************************"                                         >> ${OUTPUT_LOG}
    echo "Co-locating OrangeFS (#server_ofs1=${n_server_ofs1}, #server_ofs2=${n_server_ofs2}), rep=${rep} ..."  >> ${OUTPUT_LOG}
    diff ${OFS1_SCRIPT_DIR}/servers ${OFS2_SCRIPT_DIR}/servers                                                  >> ${OUTPUT_LOG}
    echo "************************************************************"                                         >> ${OUTPUT_LOG}
    rm -rf ${OUTPUT_LOG_IOR1}
    rm -rf ${OUTPUT_LOG_IOR2}

    ((np=${N_CLIENT_TOTAL}*20))
    if [[ ${n_server_ofs1} -ne 0 ]]
    then
      echo "---------------------------------------------------------------------------------"                  >> ${OUTPUT_LOG}
      echo OFS1 command: /home/kfeng/MPICH/bin/mpiexec -n ${np} -iface ${IFACE} -f ${IOR_CLIENT_HOSTS}\
           -prepend-rank -genv PVFS2TAB_FILE /mnt/nvme/kfeng/pvfs2tab\
           ${IOR_BIN} -a MPIIO -t ${XSIZE1_IN_KB}k -b ${BSIZE1_IN_MB}m -E -k -w -o ${DEST_FILE1}                >> ${OUTPUT_LOG}
      echo "---------------------------------------------------------------------------------"                  >> ${OUTPUT_LOG}
      /home/kfeng/MPICH/bin/mpiexec -n ${np} -iface ${IFACE} -f ${IOR_CLIENT_HOSTS}\
      -prepend-rank -genv PVFS2TAB_FILE /mnt/nvme/kfeng/pvfs2tab\
      ${IOR_BIN} -a MPIIO -t ${XSIZE1_IN_KB}k -b ${BSIZE1_IN_MB}m -E -k -w -o ${DEST_FILE1} > ${OUTPUT_LOG_IOR1} &
    fi
    pid1=$!

    if [[ ${n_server_ofs2} -ne 0 ]]
    then
      echo "---------------------------------------------------------------------------------"                  >> ${OUTPUT_LOG}
      echo OFS2 command: /home/kfeng/MPICH/bin/mpiexec -n ${np} -iface ${IFACE} -f ${IOR_CLIENT_HOSTS}\
           -prepend-rank -genv PVFS2TAB_FILE /mnt/nvme/kfeng/pvfs2tab\
           ${IOR_BIN} -a MPIIO -t ${XSIZE2_IN_MB}m -b ${BSIZE2_IN_MB}m -F -E -k -w -o ${DEST_FILE2}             >> ${OUTPUT_LOG}
      echo "---------------------------------------------------------------------------------"                  >> ${OUTPUT_LOG}
      /home/kfeng/MPICH/bin/mpiexec -n ${np} -iface ${IFACE} -f ${IOR_CLIENT_HOSTS}\
      -prepend-rank -genv PVFS2TAB_FILE /mnt/nvme/kfeng/pvfs2tab\
      ${IOR_BIN} -a MPIIO -t ${XSIZE2_IN_MB}m -b ${BSIZE2_IN_MB}m -F -E -k -w -o ${DEST_FILE2} > ${OUTPUT_LOG_IOR2} &
    fi
    pid2=$!

    wait ${pid1}
    ret1=$?
    wait ${pid2}
    ret2=$?

    echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"                    >> ${OUTPUT_LOG}

    if [[ ${ret1} != 0 || ${ret2} != 0 ]]
    then
      printf "CSV record:\t%4d\t%4d\t%10s\t%10s\n" ${n_server_ofs1} ${n_server_ofs2} "-1" "-1"              >> ${OUTPUT_RES}
      echo ""                                                                                                   >> ${OUTPUT_LOG}
      echo ""                                                                                                   >> ${OUTPUT_LOG}
      echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"                             >> ${OUTPUT_LOG}
      echo "PVFS2 error found"                                                                                  >> ${OUTPUT_LOG}
      echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"                             >> ${OUTPUT_LOG}
      echo ""                                                                                                   >> ${OUTPUT_LOG}
      echo ""                                                                                                   >> ${OUTPUT_LOG}
    else
      bw_ofs1=$(cat ${OUTPUT_LOG_IOR1} | grep "Max Write\|Max Read" | awk '{print $4}')
      bw_ofs2=$(cat ${OUTPUT_LOG_IOR2} | grep "Max Write\|Max Read" | awk '{print $4}')
      if [[ -z ${bw_ofs1} ]]
      then
        bw_ofs1=0
      fi
      if [[ -z ${bw_ofs2} ]]
      then
        bw_ofs2=0
      fi
      printf "CSV record:\t%4d\t%4d\t%10s\t%10s\n" ${n_server_ofs1} ${n_server_ofs2} ${bw_ofs1} ${bw_ofs2}  >> ${OUTPUT_RES}
    fi
    echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"                               >> ${OUTPUT_LOG}
    echo "                          Full output of IOR1"                                                        >> ${OUTPUT_LOG}
    echo "----------------------------------------------------------------------"                               >> ${OUTPUT_LOG}
    cat ${OUTPUT_LOG_IOR1}                                                                                      >> ${OUTPUT_LOG}
    echo "----------------------------------------------------------------------"                               >> ${OUTPUT_LOG}
    echo "                       End of full output of IOR1"                                                    >> ${OUTPUT_LOG}
    echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"                               >> ${OUTPUT_LOG}
    echo ""                                                                                                     >> ${OUTPUT_LOG}
    echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"                               >> ${OUTPUT_LOG}
    echo "                          Full output of IOR2"                                                        >> ${OUTPUT_LOG}
    echo "----------------------------------------------------------------------"                               >> ${OUTPUT_LOG}
    cat ${OUTPUT_LOG_IOR2}                                                                                      >> ${OUTPUT_LOG}
    echo "----------------------------------------------------------------------"                               >> ${OUTPUT_LOG}
    echo "                       End of full output of IOR2"                                                    >> ${OUTPUT_LOG}
    echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"                               >> ${OUTPUT_LOG}

    mpssh -f ${ALL_COMP_HOSTS} "sudo ${FM}" > /dev/null 2>&1
    sleep 5
    mpssh -f ${ALL_STOR_HOSTS} "sudo ${FM}" > /dev/null 2>&1
    echo ""
    echo "Co-locating OrangeFS (#server_ofs1=${n_server_ofs1}, #server_ofs2=${n_server_ofs2}), rep=${rep} finishes, sleep for 5 seconds..."
    echo ""
    sleep 5
  done
done
