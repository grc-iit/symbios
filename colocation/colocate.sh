#!/bin/bash

CWD="/home/kfeng/SuperServer/colocation"

if [[ $1 == ofs_only* ]]
then
  ${CWD}/ofs_only.sh $2
elif [[ $1 == ofs_redis* ]]
then
  ${CWD}/ofs_redis.sh $2 $3
fi
