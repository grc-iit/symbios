#!/bin/bash

mpssh > /dev/null 2>&1 || { echo >&2 "mpssh is not found.  Aborting."; exit 1; }

CWD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
SERVER_LOCAL_PATH="/mnt/hdd/kfeng"
#SERVER_LOCAL_PATH="/mnt/ssd/kfeng"
CLIENT_LOCAL_PATH="/mnt/nvme/kfeng"
mongod_shard_path=${SERVER_LOCAL_PATH}/mongod_shard
mongos_diag_data_path=${TMPFS_PATH}/mongos.diagnostic.data

MONGOD_SHARD_LOG_FILE=mongod_shard.log


SHARD_SERVER_COUNT=`cat ${CWD}/servers | wc -l`

ROUTER_SERVER_COUNT=`cat ${CWD}/clients | wc -l`

CONFIG_REPL_NAME=replconfig01
mongod_config_path=${SERVER_LOCAL_PATH}/mongod_config
TMPFS_PATH="/dev/shm"
MONGOD_CONFIG_LOG_FILE=mongod_config.log
MONGO_PORT=27017
mongos_local_path=${CLIENT_LOCAL_PATH}/mongos
MONGOS_LOG_FILE=mongos.log

MONGOS_CONF_FILE=mongos.conf
MONGOD_CONFIG_CONF_FILE=mongod_config.conf
MONGOD_SHARD_CONF_FILE=mongod_shard.conf
echo -e "${GREEN}Preparing config files${NC}"
sed -i "s|clusterRole: .*|clusterRole: configsvr|"                  ${MONGOD_CONFIG_CONF_FILE}
sed -i "s|replSetName: .*|replSetName: ${CONFIG_REPL_NAME}|"        ${MONGOD_CONFIG_CONF_FILE}
sed -i "s|dbPath:.*|dbPath: ${mongod_config_path}|"                 ${MONGOD_CONFIG_CONF_FILE}
sed -i "s|path: .*|path: ${TMPFS_PATH}/${MONGOD_CONFIG_LOG_FILE}|"  ${MONGOD_CONFIG_CONF_FILE}
sed -i "s|port: .*|port: ${MONGO_PORT}|"                            ${MONGOD_CONFIG_CONF_FILE}
sed -i "s|dbPath:.*|dbPath: ${mongos_local_path}|"                  ${MONGOS_CONF_FILE}
sed -i "s|path: .*|path: ${TMPFS_PATH}/${MONGOS_LOG_FILE}|"         ${MONGOS_CONF_FILE}
sed -i "s|port: .*|port: ${MONGO_PORT}|"                            ${MONGOS_CONF_FILE}
sed -i 's|clusterRole: .*|clusterRole: shardsvr|'                   ${MONGOD_SHARD_CONF_FILE}
sed -i "s|dbPath:.*|dbPath: ${mongod_shard_path}|"                  ${MONGOD_SHARD_CONF_FILE}
sed -i "s|path: .*|path: ${TMPFS_PATH}/${MONGOD_SHARD_LOG_FILE}|"   ${MONGOD_SHARD_CONF_FILE}
sed -i "s|port: .*|port: ${MONGO_PORT}|"                            ${MONGOD_SHARD_CONF_FILE}