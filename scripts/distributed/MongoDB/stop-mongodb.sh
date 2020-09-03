#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

CWD="$( pwd )"

SERVER_HOSTFILE=@1
MONGO_PATH=@2

mongod_config_path=${MONGO_PATH}/mongod_config
mongod_shard_path=${MONGO_PATH}/mongod_shard
mongos_local_path=${MONGO_PATH}/mongos

SERVERS="($(cat "${SERVER_HOSTFILE}"))"
#echo -e "${GREEN}Remove all${NC}"
#first_client="${CLIENTS[0]}"
#mongo --host ${first_client} -p ${MONGO_PORT} < ${CWD}/clear_db.js

echo -e "${GREEN}Stopping MongoDB servers${NC}"
for server in "${SERVERS[@]}"
do
ssh "${server}" /bin/bash << EOF
  pkill mongod
EOF
done
echo -e "${GREEN}Done stopping MongoDB${NC}"

echo -e "${GREEN}Cleaning MongoDB${NC}"
rm -rf ${MONGO_PATH}
echo -e "${GREEN}Done${NC}"
