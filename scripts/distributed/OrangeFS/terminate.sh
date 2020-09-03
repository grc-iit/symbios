#!/bin/bash

#Remeber to have env Variables for:
# ORANGEFS_KO
# ORANGEFS_PATH
# PVFS2TAB_FILE
RED='\033[0;31m'
GREEN='\033[0;32m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

CWD=$(pwd)

#Input Variables
conf_file=${1}
server_dir=${2}
client_dir=${3}
server_hostfile=${4}
client_hostfile=${5}

#General Variables
client_list=($(cat ${client_hostfile}))
server_list=($(cat ${server_hostfile}))

#Stop clients
for node in "${client_list[@]}"
do
ssh ${node} /bin/bash << EOF
echo "${GREEN}Stopping client on ${node}${NC}"
sudo /usr/sbin/kill-pvfs2-client
EOF
done

#Stop servers
for node in "${server_list[@]}"
do
ssh ${node} /bin/bash << EOF
echo "${GREEN}Killing server at ${node} ${NC}"
sudo /usr/sbin/kill-pvfs2-client
rm -rf ${server_dir}/*
killall -s SIGKILL pvfs2-server
EOF
done

echo "done"
