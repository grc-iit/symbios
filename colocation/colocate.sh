#!/bin/bash

CWD="/home/kfeng/SuperServer/colocation"

if [[ $1 == ofs_only ]]
then
  ${CWD}/ofs_only.sh $2
elif [[ $1 == ofs_redis ]]
then
  ${CWD}/ofs_redis.sh $2 $3
elif [[ $1 == ofs_redis_affinity ]]
then
  ${CWD}/ofs_redis_affinity.sh $2 $3 $4
elif [[ $1 == ofs_idle_redis ]]
then
  ${CWD}/ofs_idle_redis.sh $2
elif [[ $1 == redis_only ]]
then
  ${CWD}/redis_only.sh
elif [[ $1 == redis_idle_ofs ]]
then
  ${CWD}/redis_idle_ofs.sh
fi
