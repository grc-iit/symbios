#!/bin/bash

CWD="/home/kfeng/SuperServer/colocation"
DATA_DIR="${CWD}/results"
DELAYS=(0 10 20 40 60 80)
XSIZES=(32k 128k 512k 2048k 8192k)
TIMESTAMPS=(2019-06-10-00-03-58 2019-06-10-00-08-52 2019-06-10-00-13-47 2019-06-10-00-18-40 2019-06-10-00-23-44 2019-06-10-00-25-56 2019-06-10-00-30-45 2019-06-10-00-35-37 2019-06-10-00-40-30 2019-06-10-00-45-23 2019-06-10-00-50-12 2019-06-10-00-55-05 2019-06-10-00-59-45 2019-06-10-01-04-40 2019-06-10-01-09-36 2019-06-10-01-14-31 2019-06-10-01-19-29 2019-06-10-01-24-20 2019-06-10-01-29-07 2019-06-10-01-33-55 2019-06-10-01-38-43 2019-06-10-01-43-35 2019-06-09-23-00-03 2019-06-09-23-05-07 2019-06-09-23-10-06 2019-06-09-23-15-03 2019-06-09-23-20-03 2019-06-09-23-24-45 2019-06-09-23-29-21 2019-06-09-23-34-17 2019-06-09-23-39-21 2019-06-09-23-44-20 2019-06-09-23-49-18 2019-06-09-23-54-18 2019-06-09-23-59-14)

i=0
#for xsize in ${XSIZES[@]}
#do
  #mv ${DATA_DIR}/${TIMESTAMPS[$i]} ${DATA_DIR}/ofs_only_${xsize}
  #((i=$i+1))
#done

#for xsize in ${XSIZES[@]}
#do
  #for delay in ${DELAYS[@]}
  #do
    #mv ${DATA_DIR}/${TIMESTAMPS[$i]} ${DATA_DIR}/ofs_redis_${xsize}_${delay}
    #((i=$i+1))
  #done
#done

for xsize in ${XSIZES[@]}
do
  mv ${DATA_DIR}/ofs_only_${xsize}/instruments/PAT-Result.pdf ${DATA_DIR}/ofs_only_${xsize}/instruments/PAT-Result_ofs_only_${xsize}.pdf
done

for xsize in ${XSIZES[@]}
do
  for delay in ${DELAYS[@]}
  do
    mv ${DATA_DIR}/ofs_redis_${xsize}_${delay}/instruments/PAT-Result.pdf ${DATA_DIR}/ofs_redis_${xsize}_${delay}/instruments/PAT-Result_ofs_redis_${xsize}_${delay}.pdf
  done
done
