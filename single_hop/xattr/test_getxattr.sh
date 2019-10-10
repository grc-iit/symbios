#!/bin/bash

TEST_DIRS=(/tmp /mnt/nvme/kfeng /mnt/nvme/kfeng/pvfs2-mount)
REP=10

_self="${0##*/}"
fname=${_self%.sh}
log_file=${fname}.log

rm -f ${log_file}
for rep in `seq 1 ${REP}`
do
  echo Run ${rep} ...
  for dir in ${TEST_DIRS[@]}
  do
    ./getxattr ${dir}/testfile >> ${log_file}
  done
done
echo Done
