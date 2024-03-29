#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

mpssh > /dev/null 2>&1 || { echo >&2 "mpssh is not found.  Aborting."; exit 1; }

CWD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
SERVER_HOSTFILE=${1}
ROUTER_SERVER_HOSTFILE=${2}
CONFIG_SERVER_COUNT=${3}
DATABASE_NAME="integration"
COLLECTION_NAME="symbios"

SERVER_LOCAL_PATH="/mnt/hdd/${USER}/MongoDB"
CLIENT_LOCAL_PATH="/mnt/nvme/${USER}/MongoDB"
TMPFS_PATH="/dev/shm/${USER}/MongoDB"
mongod_config_path=${SERVER_LOCAL_PATH}/mongod_config
mongos_local_path=${CLIENT_LOCAL_PATH}/mongos
mongod_shard_path=${SERVER_LOCAL_PATH}/mongod_shard
mongos_diag_data_path=${TMPFS_PATH}/mongos.diagnostic.data
HOSTNAME_POSTFIX="-40g"
MONGOD_CONFIG_CONF_FILE=mongod_config.conf
MONGOD_CONFIG_LOG_FILE=mongod_config.log
MONGOD_SHARD_CONF_FILE=mongod_shard.conf
MONGOD_SHARD_LOG_FILE=mongod_shard.log
MONGOS_CONF_FILE=mongos.conf
MONGOS_LOG_FILE=mongos.log
MONGO_PORT=27017
SHARD_BASE_PORT=27100
CONFIG_REPL_NAME=replconfig01
SHARD_REPL_NAME=shard
SHARD_SERVER_COUNT=`cat ${SERVER_HOSTFILE} | wc -l`
SHARD_COPY_COUNT=1
ROUTER_SERVER_COUNT=`cat ${ROUTER_SERVER_HOSTFILE} | wc -l`

echo -e "${GREEN}============ Deploying MongoDB ... ============"
echo -e "${GREEN}${CWD}"
echo -e "${GREEN}======== Number of config server: ${CONFIG_SERVER_COUNT} ========"
echo -e "${GREEN}======== Number of shard server:  ${SHARD_SERVER_COUNT} ========"
echo -e "${GREEN}======== Number of router server: ${ROUTER_SERVER_COUNT} ========"

config_server_list=`head -${CONFIG_SERVER_COUNT} ${SERVER_HOSTFILE} | awk '{print $1}'`
shard_server_list=`cat ${SERVER_HOSTFILE} | awk '{print $1}'`
client_list=`cat ${ROUTER_SERVER_HOSTFILE} | awk '{print $1}'`
echo -e "${CYAN}======== Config server: \n${config_server_list} ========"
echo -e "${CYAN}======== Shard server: \n${shard_server_list} ========"
echo -e "${CYAN}======== Router server: \n${client_list} ========"

echo -e "${GREEN}Preparing config files ...${NC}"
sed -i "s|clusterRole: .*|clusterRole: configsvr|" ${MONGOD_CONFIG_CONF_FILE}
sed -i "s|replSetName: .*|replSetName: ${CONFIG_REPL_NAME}|" ${MONGOD_CONFIG_CONF_FILE}
sed -i "s|dbPath:.*|dbPath: ${mongod_config_path}|" ${MONGOD_CONFIG_CONF_FILE}
sed -i "s|path: .*|path: ${TMPFS_PATH}/${MONGOD_CONFIG_LOG_FILE}|" ${MONGOD_CONFIG_CONF_FILE}
sed -i "s|port: .*|port: ${MONGO_PORT}|" ${MONGOD_CONFIG_CONF_FILE}
sed -i "s|dbPath:.*|dbPath: ${mongos_local_path}|" ${MONGOS_CONF_FILE}
sed -i "s|path: .*|path: ${TMPFS_PATH}/${MONGOS_LOG_FILE}|" ${MONGOS_CONF_FILE}
sed -i "s|port: .*|port: ${MONGO_PORT}|" ${MONGOS_CONF_FILE}
sed -i 's|clusterRole: .*|clusterRole: shardsvr|' ${MONGOD_SHARD_CONF_FILE}
sed -i "s|dbPath:.*|dbPath: ${mongod_shard_path}|" ${MONGOD_SHARD_CONF_FILE}
sed -i "s|path: .*|path: ${TMPFS_PATH}/${MONGOD_SHARD_LOG_FILE}|" ${MONGOD_SHARD_CONF_FILE}
sed -i "s|port: .*|port: ${MONGO_PORT}|" ${MONGOD_SHARD_CONF_FILE}

if [[ -z `grep -- "${HOSTNAME_POSTFIX}" ${SERVER_HOSTFILE}` ]]
then
  sed -e "s/$/${HOSTNAME_POSTFIX}/" -i ${SERVER_HOSTFILE}
fi

if [[ -z `grep -- "${HOSTNAME_POSTFIX}" ${ROUTER_SERVER_HOSTFILE}` ]]
then
  sed -e "s/$/${HOSTNAME_POSTFIX}/" -i ${ROUTER_SERVER_HOSTFILE}
fi

config_server_list=`head -${CONFIG_SERVER_COUNT} ${SERVER_HOSTFILE} | awk '{print $1}'`
for server in ${config_server_list[@]}
do
  echo -e "${CYAN}${server}${NC}"
  ssh ${server} "mkdir -p ${mongod_config_path};mkdir -p ${TMPFS_PATH}" ; rsync -az ${CWD}/${MONGOD_CONFIG_CONF_FILE} ${server}:${SERVER_LOCAL_PATH}/${MONGOD_CONFIG_CONF_FILE}
done
wait
client_list=`cat ${ROUTER_SERVER_HOSTFILE} | awk '{print $1}'`
for client in ${client_list[@]}
do
  echo -e "${CYAN}${client}${NC}"
  ssh ${client} "mkdir -p ${mongos_local_path};mkdir -p ${TMPFS_PATH}" ; rsync -az ${CWD}/${MONGOS_CONF_FILE} ${client}:${CLIENT_LOCAL_PATH}/${MONGOS_CONF_FILE}
done
wait
shard_server_list=`cat ${SERVER_HOSTFILE} | awk '{print $1}'`
for server in ${shard_server_list[@]}
do
  echo -e "${CYAN}${server}${NC}"
  ssh ${server} "mkdir -p ${mongod_shard_path};mkdir -p ${TMPFS_PATH}" ; rsync -az ${CWD}/${MONGOD_SHARD_CONF_FILE} ${server}:${SERVER_LOCAL_PATH}/${MONGOD_SHARD_CONF_FILE}
done
wait

echo -e "${GREEN}Starting config nodes ...${NC}"
for config_server in ${config_server_list[@]}
do
  echo -e "${CYAN}${config_server}${NC}"
  ssh ${config_server} "numactl --interleave=all mongod --config ${SERVER_LOCAL_PATH}/${MONGOD_CONFIG_CONF_FILE} --fork"
done
wait
sleep 5

echo -e "${GREEN}Initializing config replica set ...${NC}"
first_config_server=`head -1 ${SERVER_HOSTFILE}`
second_config_server=`head -2 ${SERVER_HOSTFILE} | tail -1`
sed -i "s|_id : 0, host : \".*|_id : 0, host : \"${first_config_server}:${MONGO_PORT}\" },|" conf_replica_init.js
sed -i "s|_id : 1, host : \".*|_id : 1, host : \"${second_config_server}:${MONGO_PORT}\" }|" conf_replica_init.js
mongo --host ${first_config_server} --port ${MONGO_PORT} < conf_replica_init.js > conf_replica_init.log
cat conf_replica_init.log | grep -i ok
mongo --host ${first_config_server} --port ${MONGO_PORT} --eval "rs.isMaster()" > conf_replica_init.log
cat conf_replica_init.log | grep -i "ismaster\|configsvr"
mongo --host ${first_config_server} --port ${MONGO_PORT} --eval "rs.status()" > conf_replica_init.log
cat conf_replica_init.log | grep -i "ok\|\"name\"\|stateStr"

SHARD_BASE_PORT_BAKE=${SHARD_BASE_PORT}
echo -e "${GREEN}Starting shard nodes ...${NC}"
shard_server_list=`head -${SHARD_SERVER_COUNT} ${SERVER_HOSTFILE} | awk '{print $1}'`
step=1
for shard_server in ${shard_server_list}
do
  ssh ${shard_server} "sed -i -e 's|replSetName: .*|replSetName: ${SHARD_REPL_NAME}${step}|' -e 's|port: .*|port: ${SHARD_BASE_PORT}|' ${SERVER_LOCAL_PATH}/${MONGOD_SHARD_CONF_FILE}"
  ((SHARD_BASE_PORT = SHARD_BASE_PORT + 1))
  ((step=step+1))
done
wait
for shard_server in ${shard_server_list}
do
  ssh ${shard_server} "numactl --interleave=all mongod --config ${SERVER_LOCAL_PATH}/${MONGOD_SHARD_CONF_FILE} --fork"
done
wait
sleep 5

shard_server_list=`head -${SHARD_SERVER_COUNT} ${SERVER_HOSTFILE} | awk '{print $1}'`
echo -e "${GREEN}Initializing shard replica set ...${NC}"
echo -e "${GREEN}Preparing shards to mongos/query router ...${NC}"
truncate -s 0 add_shard_to_mongos.js
count=0
for shard_server in ${shard_server_list}
do
  truncate -s 0 shard_replica_init.js
  printf "sh.addShard(\"${SHARD_REPL_NAME}$((count+1))/" >> add_shard_to_mongos.js
  printf "rs.initiate(\n{\n" > shard_replica_init.js
  printf "\t_id : \"${SHARD_REPL_NAME}$((count+1))\",\n" >> shard_replica_init.js
  printf "\tmembers: [\n" >> shard_replica_init.js

  number=0
  for ((i=0;i<"SHARD_COPY_COUNT";i++))
  do
    nodeid=$(((SHARD_SERVER_COUNT+count-i)%SHARD_SERVER_COUNT))
    current_server=`sed -n $((nodeid+1))p ${SERVER_HOSTFILE} | awk '{print $1}'`
    if [[ ${i} != $((SHARD_COPY_COUNT-1)) ]]
    then
      printf "\t\t{ _id :a %s, host : \"%s\" },\n" ${number} ${current_server}:$((SHARD_BASE_PORT_BAKE+count)) >> shard_replica_init.js
      printf "${current_server}:$((SHARD_BASE_PORT_BAKE+count))," >> add_shard_to_mongos.js
    else
      printf "\t\t{ _id : %s, host : \"%s\" }\n" ${number} ${current_server}:$((SHARD_BASE_PORT_BAKE+count)) >> shard_replica_init.js
      printf "${current_server}:$((SHARD_BASE_PORT_BAKE+count))\")\n" >> add_shard_to_mongos.js
    fi
      number=$((number+1))
  done

  printf "\t]\n}\n)\n" >> shard_replica_init.js
  cat shard_replica_init.js
  mongo --host ${current_server} --port $((SHARD_BASE_PORT_BAKE+count)) < shard_replica_init.js > shard_replica_init.log
  echo -e "${CYAN}Finish configuring shard server ${current_server}:$((SHARD_BASE_PORT_BAKE+count))${NC}"
  count=$((count+1))
done
cat shard_replica_init.log | grep -i ok

echo -e "${GREEN}Starting router nodes ...${NC}"
router_server_list=`head -${ROUTER_SERVER_COUNT} ${ROUTER_SERVER_HOSTFILE} | awk '{print $1}'`
mongos_cmd="mongos --configdb \"${CONFIG_REPL_NAME}/"
for config_server in ${config_server_list[@]}
do
  mongos_cmd="${mongos_cmd}${config_server}:${MONGO_PORT},"
done
mongos_cmd=`echo ${mongos_cmd} | sed 's/,$/"/'`
mongos_cmd="${mongos_cmd} --config ${CLIENT_LOCAL_PATH}/${MONGOS_CONF_FILE} --fork"
echo $mongos_cmd
for router_server in ${router_server_list[@]}
do
  ssh ${router_server} "${mongos_cmd}"
done
echo -e "${CYAN}waiting for router server to complete start ...${NC}"
wait
sleep 5

echo -e "${GREEN}Adding shards to mongos/query router ...${NC}"
truncate -s 0 add_shard_to_mongos.log
mongo --host ${router_server} --port ${MONGO_PORT} < add_shard_to_mongos.js >> add_shard_to_mongos.log
cat add_shard_to_mongos.log | grep -i ok

echo -e "${GREEN}Enabling sharding ...${NC}"
truncate -s 0 enable_sharding.log
mongo --host ${router_server} --port ${MONGO_PORT} < enable_sharding.js >> enable_sharding.log
cat enable_sharding.log | grep -i ok

echo -e "${GREEN}Checking mongod ...${NC}"
mpssh -f ${SERVER_HOSTFILE} 'pgrep -la mongod' | sort
echo -e "${GREEN}Checking mongos ...${NC}"
mpssh -f ${ROUTER_SERVER_HOSTFILE} 'pgrep -la mongos' | sort

echo -e "${GREEN}Done starting MongoDB${NC}"
