#!/bin/bash

dev_type=$1
conf_id=$2
conf_file=$3
server_hostfile=$4
client_hostfile=$5
script_root=$6
build_dir=$7

client_dir=/mnt/nvme/${USER}/write/generic
PROCS=(1 2 4 8 16 32 40)

echo "Deploying"
bash ${script_root}/deploy.sh ${conf_file} /mnt/${dev_type}/${USER}/orangefs ${client_dir} ${server_hostfile} ${client_hostfile}
echo "Deployed"
cd ${build_dir}
for PROC in ${PROCS[@]}; do
  echo "Tests for NPROCS=${PROC}"
  rm -rf ${client_dir}/*
  ctest --verbose -R prealloc_${PROC}_orangefs_*
  ctest --verbose -R aresbm_io_${PROC}_orangefs_[a-zA-Z0-9.]+_[a-zA-Z0-9.]+_[a-zA-Z0-9.]+_[a-zA-Z0-9.]+_[a-zA-Z0-9.]+_[a-zA-Z0-9.]+_[a-zA-Z0-9.]+_${conf_id}
  ctest --verbose -R aresbm_md_${PROC}_orangefs_[a-zA-Z0-9.]+_[a-zA-Z0-9.]+_[a-zA-Z0-9.]+_${conf_id}
  rm -rf ${client_dir}/*
done
echo "Terminating"
bash ${script_root}/terminate.sh ${conf_file} /mnt/${dev_type}/${USER}/orangefs ${client_dir} ${server_hostfile} ${client_hostfile}
echo "Terminated"