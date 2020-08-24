#!/bin/bash

CWD="$(cd "$(dirname "${BASH_SOURCE[0]}")" > /dev/null 2>&1 && pwd)"
TEST_POSIX_DIRS=(/tmp /mnt/nvme/kfeng /mnt/nvme/kfeng/pvfs2-mount)
#TEST_OFS_DIRS=(pvfs2://mnt/nvme/kfeng/pvfs2-mount)
TEST_OFS_DIRS=(/mnt/nvme/kfeng/pvfs2-mount)
#EXECUTABLES=(${CWD}/header_mpi_direct_ifce ${CWD}/header_mpi_non_direct_ifce)
#EXECUTABLES=(${CWD}/setxattr_direct_ifce ${CWD}/setxattr_non_direct_ifce)
EXECUTABLES=(${CWD}/setxattr)
CLIENT_HOST_FILE=${CWD}/clients
SERVER_HOST_FILE=${CWD}/servers
PPN=(10 20 40)
REP=3

_self="${0##*/}"
fname=${_self%.sh}
st_log_file=${fname}_st.log
mt_log_file=${fname}_mt.log
n_client=$(cat ${CLIENT_HOST_FILE} | wc -l)
n_server=$(cat ${SERVER_HOST_FILE} | wc -l)

rm -f ${st_log_file}
rm -f ${mt_log_file}

echo "Testing with ${n_server} servers and ${n_client} clients ..."

#echo "Testing shadow creation (single-thread) ..."
#for dir in ${TEST_POSIX_DIRS[@]}
#do
  #for rep in `seq 1 ${REP}`
  #do
    #echo "Run ${rep} ..."
    #./setxattr ${dir}/testfile >> ${st_log_file}
    #./header ${dir}/testfile >> ${st_log_file}
    #./header_mpi ${dir}/testfile >> ${st_log_file}
  #done
#done

echo "Testing shadow creation (multi-thread) ..."
for dir in ${TEST_OFS_DIRS[@]}
do
  if [[ ${dir} == "pvfs2"* ]]
  then
    pvfs2-rm -rf ${dir}/testfile*
  else
    rm -rf ${dir}/testfile*
  fi

  for ppn in ${PPN[@]}
  do
    ((np=${ppn}*${n_client}))
    for exe in ${EXECUTABLES[@]}
    do
      for rep in `seq 1 ${REP}`
      do
        echo "Run ${rep} ..."
        echo "mpiexec -n ${np} -f ${CLIENT_HOST_FILE} ${exe} ${dir}/testfile" >> ${mt_log_file}
        mpiexec -n ${np} -f ${CLIENT_HOST_FILE} ${exe} ${dir}/testfile >> ${mt_log_file}
        mpssh -f ${SERVER_HOST_FILE} 'sudo fm' > /dev/null
      done
    done
  done
done

echo "Done"
