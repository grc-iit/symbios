#!/bin/bash

CWD="/home/kfeng/SuperServer/colocation"
PAT_DIR="/home/kfeng/pkg_src/PAT"
PAT_COLL_DIR="${PAT_DIR}/PAT-collecting-data"
PAT_PROC_DIR="${PAT_DIR}/PAT-post-processing"
PAT_DATA_DIR="${PAT_COLL_DIR}/results"
test_run_name=$1

cd ${PAT_DATA_DIR}
date=`date +"%Y-%m-%d"`
timestamp=`ls -t | head -1`
if [[ ${timestamp} == "2019"* ]]
then
  cd ${PAT_PROC_DIR}
  data_dir="\/home\/kfeng\/pkg_src\/PAT\/PAT-collecting-data\/results"
  echo "Data is in ${data_dir}/${timestamp}"
  sed -i "s/\/.*\/instruments/${data_dir}\/${timestamp}\/instruments/g" config.xml
  ./pat-post-process.py
  mv ${PAT_DATA_DIR}/${timestamp} ${PAT_DATA_DIR}/${test_run_name}
  mv ${PAT_DATA_DIR}/${test_run_name}/instruments/PAT-Result.pdf ${PAT_DATA_DIR}/${test_run_name}/instruments/PAT-Result-${test_run_name}.pdf
else
  echo "Something is wrong, skip analysis ..."
fi
cd ${CWD}
