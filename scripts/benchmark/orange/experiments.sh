#!/bin/bash

dev_type=$1
conf_id=$2
conf_file=$3
server_hostfile=$4
script_root=$5
build_dir=$6

client_dir=/mnt/nvme/${USER}/write/generic
PROCS=(1 2 4 8 16 32 40)

bash ${script_root}/deploy.sh ${conf_file} /mnt/${dev_type}/${USER}/orangefs ${client_dir} ${server_hostfile}
cd ${build_dir}
for PROC in ${PROCS[@]}; do
  ctest --verbose -R prealloc_${PROC}_orangefs_*
  ctest --verbose -R aresbm_io_${PROC}_orangefs_[a-zA-Z0-9]+_[a-zA-Z0-9]+_[a-zA-Z0-9]+_[a-zA-Z0-9]+_[a-zA-Z0-9]+_[a-zA-Z0-9]+_[a-zA-Z0-9]+_${conf_id}
  ctest --verbose -R aresbm_md_${PROC}_orangefs_[a-zA-Z0-9]+_[a-zA-Z0-9]+_[a-zA-Z0-9]+_${conf_id}
  #ctest -N -R prealloc_${PROC}_orangefs_*
  #ctest -N -R aresbm_io_${PROC}_orangefs_[a-zA-Z0-9]+_[a-zA-Z0-9]+_[a-zA-Z0-9]+_[a-zA-Z0-9]+_[a-zA-Z0-9]+_[a-zA-Z0-9]+_[a-zA-Z0-9]+_${conf_id}
  #ctest -N -R aresbm_md_${PROC}_orangefs_[a-zA-Z0-9]+_[a-zA-Z0-9]+_[a-zA-Z0-9]+_${conf_id}
done
bash ${script_root}/terminate.sh ${conf_file} /mnt/${dev_type}/${USER}/orangefs ${client_dir} ${server_hostfile}
