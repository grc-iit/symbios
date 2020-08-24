#!/bin/bash

CWD="/home/kfeng/SuperServer/colocation"
PAT_DIR="/home/kfeng/pkg_src/PAT"
PAT_DATA_DIR="${CWD}/results_local_redis"
OUTPUT_DIR="${CWD}/results_local_redis/stitched"
OUTPUT_TMP_DIR="${CWD}/results_local_redis/stitched/tmp"
DELAYS=(0 5 10 15 20 25 30 35 40)
XSIZES=(32k 128k 512k 2048k 8192k 16384k)
NUM_PAGES=7

mkdir -p ${OUTPUT_TMP_DIR}
cd ${PAT_DATA_DIR}
# Extract
for xsize in ${XSIZES[@]}
do
  # For OrangeFS only
  for page in `seq 1 ${NUM_PAGES}`
  do
    pdftk ofs_only_${xsize}/instruments/PAT-Result-ofs_only_${xsize}.pdf cat ${page} output ${OUTPUT_TMP_DIR}/ofs_only_${xsize}_p${page}.pdf
  done

  # For mixed OrangeFS and Redis
  for delay in ${DELAYS[@]}
  do
    for page in `seq 1 ${NUM_PAGES}`
    do
      pdftk ofs_redis_${xsize}_${delay}/instruments/PAT-Result-ofs_redis_${xsize}_${delay}.pdf cat ${page} output ${OUTPUT_TMP_DIR}/ofs_redis_${xsize}_${delay}_p${page}.pdf
    done
  done
done

# Stitch
cd ${OUTPUT_TMP_DIR}
for xsize in ${XSIZES[@]}
do
  for page in `seq 1 ${NUM_PAGES}`
  do
    file_list=ofs_only_${xsize}_p${page}.pdf
    for delay in ${DELAYS[@]}
    do
      file_list="${file_list} ofs_redis_${xsize}_${delay}_p${page}.pdf"
    done
    #echo ${file_list}
    pdftk ${file_list} cat output ${OUTPUT_DIR}/${xsize}_p${page}.pdf
  done
done

cd ${CWD}
