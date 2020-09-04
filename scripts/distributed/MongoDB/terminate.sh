#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

CWD="$( pwd )"

SERVER_HOSTFILE=${1}
CLIENT_HOSTFILE=${2}
SERVER_LOCAL_PATH="/mnt/hdd/${USER}/MongoDB"
CLIENT_LOCAL_PATH="/mnt/nvme/${USER}/MongoDB"
TMPFS_PATH="/dev/shm/${USER}/MongoDB"

SERVERS=($(cat ${SERVER_HOSTFILE}))
CLIENTS=($(cat ${CLIENT_HOSTFILE}))
#echo -e "${GREEN}Remove all${NC}"
#first_client="${CLIENTS[0]}"
#mongo --host ${first_client} -p ${MONGO_PORT} < ${CWD}/clear_db.js

echo -e "${GREEN}Stopping MongoDB servers${NC}"
echo -e "${CYAN}Servers :${NC}${SERVERS[*]}"
for server in "${SERVERS[@]}"
do
ssh "${server}" /bin/bash << EOF
  pkill mongod
  rm -rf ${SERVER_LOCAL_PATH}/*
  rm -rf ${TMPFS_PATH}/*
EOF
done

echo -e "${GREEN}Stopping MongoDB Router servers${NC}"
echo -e "${CYAN}Router Servers :${NC}${CLIENTS[*]}"
for client in "${CLIENTS[@]}"
do
ssh "${client}" /bin/bash << EOF
  pkill mongos
  rm -rf ${CLIENT_LOCAL_PATH}/*
  rm -rf ${TMPFS_PATH}/*
EOF
done
echo -e "${GREEN}Done stopping MongoDB${NC}"
