#!/bin/bash

CWD="/home/kfeng/SuperServer/internal_component/engine"
SCRIPT_DIR="/home/kfeng/pkg_src/Utility-scripts"
PFS1_SCRIPT_DIR="${SCRIPT_DIR}/OrangeFS"
PFS2_SCRIPT_DIR="${SCRIPT_DIR}/OrangeFS2"
REDIS_SCRIPT_DIR="${SCRIPT_DIR}/Redis"
MONGO_SCRIPT_DIR="${SCRIPT_DIR}/MongoDB"
CLIENT_HOSTS="${CWD}/clients"
SERVER_HOSTS="${CWD}/servers"
ALL_HOSTS=$(cat ${CWD}/clients ${CWD}/servers > ${CWD}/all_nodes)
REDIS_HOSTS="${CWD}/redis_hosts"
first_client=`head -1 ${CLIENT_HOSTS}`
first_server=`head -1 ${SERVER_HOSTS}`
PFS1_MOUNT_POINT="/mnt/nvme/kfeng/pvfs2-mount"
PFS2_MOUNT_POINT="/mnt/nvme/kfeng/pvfs2-mount2"

REQ_SIZE_LIST=(16384 32768 65536 131072 262144 524288 1048576 2097152 4194304)
#REQ_SIZE_LIST=(4194304)
TARGET_STOR_LIST=("PFS1" "PFS2" "REDIS" "MONGO")
#TARGET_STOR_LIST=("PFS1") # "REDIS" "MONGO")
IO_MODE_LIST=("WRITE" "READ")
TRACE_DIR="${CWD}/traces/synthetic/benchmark"
EXEC_BIN="${CWD}/test_engine"
IFACE="enp47s0"
OUTPUT_LOG="${CWD}/benchmark.log"
OUTPUT_RES="${CWD}/result.log"
NP=24
REP=2

# Remove all data on storage systems
remove_pfs_data()
{
  pfs_id=$1

  if [[ ${pfs_id} -eq 1 ]]
  then
    remote_dir=${PFS1_MOUNT_POINT}
  elif [[ ${pfs_id} -eq 2 ]]
  then
    remote_dir=${PFS2_MOUNT_POINT}
  fi
  ssh ${first_client} "rm -rf ${remote_dir}/*"
}

remove_redis_data()
{
  cd ${REDIS_SCRIPT_DIR}
  ./flushall.sh
  cd ${CWD}
}

remove_mongo_data()
{
  cd ${MONGO_SCRIPT_DIR}
  ./removeall.sh
  cd ${CWD}
}

flush_cache()
{
  mpssh -f ${CLIENT_HOSTS} "sudo fm" > /dev/null 2>&1
  sleep 2
  mpssh -f ${SERVER_HOSTS} "sudo fm" > /dev/null 2>&1
  sleep 3
}

remove_all_data()
{
  remove_pfs_data 1
  remove_pfs_data 2
  remove_redis_data
  remove_mongo_data
  sleep 5
}

# Start storage systems
start_pfs()
{
  pfs_id=$1

  if [[ ${pfs_id} -eq 1 ]]
  then
    cd ${PFS1_SCRIPT_DIR}
  elif [[ ${pfs_id} -eq 2 ]]
  then
    cd ${PFS2_SCRIPT_DIR}
  fi
  ./genconfig.sh
  ./start-server.sh
  ./start-client.sh
  cd ${CWD}
}

start_redis()
{
  cd ${REDIS_SCRIPT_DIR}
  ./start.sh
  cd ${CWD}
}

start_mongo()
{
  cd ${MONGO_SCRIPT_DIR}
  ./start-mongodb.sh
  cd ${CWD}
}

start_all()
{
  start_pfs 1
  start_pfs 2
  start_redis
  start_mongo
}

# Clean storage systems
clean_all()
{
  cd ${PFS1_SCRIPT_DIR}
  ./clean.sh &
  cd ${PFS2_SCRIPT_DIR}
  ./clean.sh &
  cd ${REDIS_SCRIPT_DIR}
  ./clean.sh &
  cd ${MONGO_SCRIPT_DIR}
  ./clean.sh &
  wait
  cd ${CWD}
}

clean_all
start_all
mv -f ${OUTPUT_LOG} ${OUTPUT_LOG}.bak
mv -f ${OUTPUT_RES} ${OUTPUT_RES}.bak

echo "Starting test ..."
echo "Benchmark starts: `date +"%Y-%m-%d-%H-%M-%S-%N"`"     >> ${OUTPUT_LOG}
echo "**********************************************"       >> ${OUTPUT_LOG}
printf "Request sizes:\t\t\t\t\t\t\t${REQ_SIZE_LIST}\n"     >> ${OUTPUT_LOG}
printf "Target storage systems:\t\t\t${TARGET_STOR_LIST}\n" >> ${OUTPUT_LOG}
printf "I/O modes:\t\t\t\t\t\t\t\t\t${IO_MODE_LIST}\n"      >> ${OUTPUT_LOG}
echo "**********************************************"       >> ${OUTPUT_LOG}

for req_size in ${REQ_SIZE_LIST[@]}
do
  for target_stor in ${TARGET_STOR_LIST[@]}
  do
    for io_mode in ${IO_MODE_LIST[@]}
    do
      for rep in `seq 1 ${REP}`
      do
        if [[ ${io_mode} == "WRITE" ]]
        then
          remove_all_data
        fi

        io_mode_lower=$(echo ${io_mode} | tr '[:upper:]' '[:lower:]')
        trace_file=${TRACE_DIR}/${io_mode_lower}_${req_size}.csv
        echo ""
        echo "Testing ${io_mode_lower}ing to/from ${target_stor} using traces from ${trace_file} ..."
        echo ""

        echo "====================================================================="    >> ${OUTPUT_LOG}
        printf "			Request size:\t\t\t${req_size}\n"                                 >> ${OUTPUT_LOG}
        printf "			Target storage:\t\t${target_stor}\n"                              >> ${OUTPUT_LOG}
        printf "			I/O mode:\t\t\t\t\t${io_mode_lower}\n"                            >> ${OUTPUT_LOG}
        printf "			Trace file:\t\t\t\t${trace_file}\n"                               >> ${OUTPUT_LOG}
        echo "====================================================================="    >> ${OUTPUT_LOG}

        flush_cache
        echo Command: /home/kfeng/MPICH/bin/mpiexec\
             -n ${NP} -f ${CLIENT_HOSTS} -iface ${IFACE} --prepend-rank\
             ${EXEC_BIN} -f ${trace_file} -r ${REDIS_HOSTS} -t ${target_stor}           >> ${OUTPUT_LOG}
        output=$(timeout -s 9 600 /home/kfeng/MPICH/bin/mpiexec\
                 -n ${NP} -f ${CLIENT_HOSTS} -iface ${IFACE} --prepend-rank\
                 ${EXEC_BIN} -f ${trace_file} -r ${REDIS_HOSTS} -t ${target_stor} 2>&1)

        ret=$?
        if [[ ${ret} != 0 ]]
        then
          printf "CSV record:\t%10d\t%6s\t%6s\t%10s\t%10s\n"\
                  ${req_size} ${target_stor} ${io_mode_lower} "-1" "-1"             >> ${OUTPUT_RES}
          echo ""                                                                       >> ${OUTPUT_LOG}
          echo ""                                                                       >> ${OUTPUT_LOG}
          echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" >> ${OUTPUT_LOG}
          echo "Errors found"                                                           >> ${OUTPUT_LOG}
          echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" >> ${OUTPUT_LOG}
          echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
          echo "Errors found"
          echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
          echo ""                                                                       >> ${OUTPUT_LOG}
          echo ""                                                                       >> ${OUTPUT_LOG}
        else
          bw=$(echo "${output}" | grep 'Bandwidth' | awk '{print $3}')
          total_time=$(echo "${output}" | grep 'Duration' | awk '{print $3}')
          printf "CSV record:\t%10d\t%6s\t%6s\t%10s\t%10s\n"\
                  ${req_size} ${target_stor} ${io_mode_lower} ${total_time} ${bw}   >> ${OUTPUT_RES}
        fi

        echo ""                                                                         >> ${OUTPUT_LOG}
        echo "Full output:"                                                             >> ${OUTPUT_LOG}
        echo "${output}"                                                                >> ${OUTPUT_LOG}
      done
      echo ""
      echo "Testing ${io_mode_lower}ing to/from ${target_stor} using traces from ${trace_file} finishes, sleep 5 seconds ..."
      echo ""
      sleep 5
    done # End of io_mode loop
    remove_all_data
  done # End of target_stor loop
done # End of req_size loop

ssh ${first_client} 'ls -lh /mnt/nvme/kfeng/pvfs2-mount/' >> ${OUTPUT_RES}

echo "Benchmark ends: `date +'%Y-%m-%d-%H-%M-%S-%N'`"                                >> ${OUTPUT_LOG}

echo "Done"
