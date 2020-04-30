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
N_SERVER_OFS1=24
N_SERVER_OFS2=24
#TIME_DELAY_IN_US_LIST=(0 500000 1000000 1500000 2000000 2500000 3000000 3500000 4000000 4500000 5000000 5500000 6000000 6500000 7000000 7500000 8000000)
TIME_DELAY_IN_US_LIST=(0) # 500000 1000000 1500000 2000000 2500000 3000000)
XSIZE1_IN_KB=128
#XSIZE1_IN_MB=4
XSIZE2_IN_MB=4
#XSIZE2_IN_KB=128
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
    cat ${ALL_STOR_HOSTS} | head -n ${n_server_ofs2} > ${OFS2_SCRIPT_DIR}/servers
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

mv -f ${OUTPUT_LOG} ${OUTPUT_LOG}.bak
mv -f ${OUTPUT_RES} ${OUTPUT_RES}.bak
echo "Cleaning up ..."
cleanup
echo "Generating host files ..."
gen_ofs_host_file ${N_SERVER_OFS1} ${N_SERVER_OFS2}
echo "Starting OrangeFS ..."
start_ofs ${N_SERVER_OFS1} ${N_SERVER_OFS2}
rm -rf ${OUTPUT_LOG_IOR1}
rm -rf ${OUTPUT_LOG_IOR2}
cd ${CWD}
#for overlap_ratio in ${TIME_OVERLAP_RATIO_LIST}
for time_delay_in_us in ${TIME_DELAY_IN_US_LIST[@]}
do
  for rep in `seq 1 ${REP}`
  do
    echo ""                                                                                                     >> ${OUTPUT_LOG}
    echo "************************************************************"                                         >> ${OUTPUT_LOG}
    echo "Co-locating OrangeFS (#server_ofs1=${N_SERVER_OFS1}, #server_ofs2=${N_SERVER_OFS2}), rep=${rep} ..."  >> ${OUTPUT_LOG}
    diff ${OFS1_SCRIPT_DIR}/servers ${OFS2_SCRIPT_DIR}/servers                                                  >> ${OUTPUT_LOG}
    echo "************************************************************"                                         >> ${OUTPUT_LOG}

    ((np=${N_CLIENT_TOTAL}*20))
    echo "---------------------------------------------------------------------------------"                  >> ${OUTPUT_LOG}
    echo OFS1 command: /home/kfeng/MPICH/bin/mpiexec -n ${np} -iface ${IFACE} -f ${IOR_CLIENT_HOSTS}\
         -prepend-rank -genv PVFS2TAB_FILE /mnt/nvme/kfeng/pvfs2tab\
         ${IOR_BIN} -a MPIIO -t ${XSIZE1_IN_KB}k -b ${BSIZE1_IN_MB}m -E -k -w -o ${DEST_FILE1}                >> ${OUTPUT_LOG}
    echo "---------------------------------------------------------------------------------"                  >> ${OUTPUT_LOG}
    /home/kfeng/MPICH/bin/mpiexec -n ${np} -iface ${IFACE} -f ${IOR_CLIENT_HOSTS}\
    -prepend-rank -genv PVFS2TAB_FILE /mnt/nvme/kfeng/pvfs2tab\
    ${IOR_BIN} -a MPIIO -t ${XSIZE1_IN_KB}k -b ${BSIZE1_IN_MB}m -E -k -w -o ${DEST_FILE1} > ${OUTPUT_LOG_IOR1}
    pid1=$!
    ret1=$?

    usleep ${time_delay_in_us}
    echo "---------------------------------------------------------------------------------"                  >> ${OUTPUT_LOG}
    echo OFS2 command: /home/kfeng/MPICH/bin/mpiexec -n ${np} -iface ${IFACE} -f ${IOR_CLIENT_HOSTS}\
         -prepend-rank -genv PVFS2TAB_FILE /mnt/nvme/kfeng/pvfs2tab\
         ${IOR_BIN} -a MPIIO -t ${XSIZE2_IN_MB}m -b ${BSIZE2_IN_MB}m -F -E -k -w -o ${DEST_FILE2}             >> ${OUTPUT_LOG}
    echo "---------------------------------------------------------------------------------"                  >> ${OUTPUT_LOG}
    /home/kfeng/MPICH/bin/mpiexec -n ${np} -iface ${IFACE} -f ${IOR_CLIENT_HOSTS}\
    -prepend-rank -genv PVFS2TAB_FILE /mnt/nvme/kfeng/pvfs2tab\
    ${IOR_BIN} -a MPIIO -t ${XSIZE2_IN_MB}m -b ${BSIZE2_IN_MB}m -F -E -k -w -o ${DEST_FILE2} > ${OUTPUT_LOG_IOR2}
    pid2=$!
    ret2=$?

    #wait ${pid1}
    #ret1=$?

    echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"                    >> ${OUTPUT_LOG}

    if [[ ${ret1} != 0 || ${ret2} != 0 ]]
    then
      printf "CSV record:\t%10d\t%10s\t%10s\t%10s\n" ${time_delay_in_us} "-1" "-1"                          >> ${OUTPUT_RES}
      echo ""                                                                                                   >> ${OUTPUT_LOG}
      echo ""                                                                                                   >> ${OUTPUT_LOG}
      echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"                             >> ${OUTPUT_LOG}
      echo "PVFS2 error found"                                                                                  >> ${OUTPUT_LOG}
      echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"                             >> ${OUTPUT_LOG}
      echo ""                                                                                                   >> ${OUTPUT_LOG}
      echo ""                                                                                                   >> ${OUTPUT_LOG}
    else
      bw_ofs1=$(cat ${OUTPUT_LOG_IOR1} | grep "Max Write\|Max Read" | awk '{print $4}')
      if [[ -z ${bw_ofs1} ]]
      then
        bw_ofs1=0
      fi
      bw_ofs2=$(cat ${OUTPUT_LOG_IOR2} | grep "Max Write\|Max Read" | awk '{print $4}')
      if [[ -z ${bw_ofs2} ]]
      then
        bw_ofs2=0
      fi
      printf "CSV record:\t%10d\t%10s\t%10s\t%10s\n" ${time_delay_in_us} ${bw_ofs1} ${bw_ofs2}              >> ${OUTPUT_RES}
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
    echo "Co-locating OrangeFS (#server_ofs1=${N_SERVER_OFS1}, #server_ofs2=${N_SERVER_OFS2}), rep=${rep} finishes, sleep for 5 seconds..."
    echo ""
    sleep 5
  done
done
