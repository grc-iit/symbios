#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

CWD="$( pwd )"

SERVER_HOSTFILE=@1
CLIENT_HOSTFILE=@1

SERVERS="($(cat "${SERVER_HOSTFILE}"))"
SERVERS="($(cat "${CLIENT_HOSTFILE}"))"

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

echo -e "${GREEN}Stopping MongoDB clients${NC}"
for client in "${CLIENTS[@]}"
do
ssh "${client}" /bin/bash << EOF
  pkill mongos
EOF
done
echo -e "${GREEN}Done stopping MongoDB${NC}"

echo -e "${GREEN}Cleaning MongoDB${NC}"
echo -e "${GREEN}Server${NC}"
for server in "${SERVERS[@]}"
do
ssh "${server}" /bin/bash << EOF
  rm -rf "${mongod_config_path}" "${mongod_shard_path}" > /dev/null
  rm -rf ${SERVER_LOCAL_PATH}/${MONGOD_CONFIG_CONF_FILE} ${SERVER_LOCAL_PATH}/${MONGOD_SHARD_CONF_FILE}" > /dev/null
  rm -rf ${TMPFS_PATH}/${MONGOD_CONFIG_LOG_FILE} ${TMPFS_PATH}/${MONGOD_SHARD_LOG_FILE}" > /dev/null
EOF
done
echo -e "${GREEN}Clients${NC}"
for client in "${CLIENTS[@]}"
do
ssh "${client}" /bin/bash << EOF
  rm -rf "${mongos_local_path}" > /dev/null
  rm -rf ${CLIENT_LOCAL_PATH}/${MONGOS_CONF_FILE}" > /dev/null
  rm -rf ${TMPFS_PATH}/${MONGOS_LOG_FILE} ${mongos_diag_data_path}" > /dev/null
EOF
done

echo -e "${GREEN}Done cleaning MongoDB${NC}"
