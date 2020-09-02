#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

CWD="$( pwd )"
CONFIG_SERVER_HOSTFILE=@1
SHARD_SERVER_HOSTFILE=@2
ROUTER_SERVER_HOSTFILE=@3

CONFIG_SERVERS="($(cat "${CONFIG_SERVER_HOSTFILE}"))"
SHARD_SERVERS="($(cat "${SHARD_SERVER_HOSTFILE}"))"
ROUTER_SERVERS="($(cat "${ROUTER_SERVER_HOSTFILE}"))"
CLIENTS="($(cat "${CLIENT_HOSTFILE}"))"

SHARD_COPY_COUNT=1
SHARD_REPL_NAME=shard
SHARD_COPY_COUNT=1
SHARD_BASE_PORT=27100
CONFIG_SERVER_COUNT=2

echo -e "${GREEN}============ Deploying MongoDB ============${NC}"

mkdir -p ${mongod_config_path}
mkdir -p ${mongod_shard_path}

echo -e "${GREEN}Starting config nodes${NC}"
for config_server in "${CONFIG_SERVER_HOSTFILE[@]}"
do
ssh "${config_server}" /bin/bash << EOF
  mongod --config ${SERVER_LOCAL_PATH}/${MONGOD_CONFIG_CONF_FILE} --fork &
EOF
done

echo -e "${GREEN}Initializing config replica set${NC}"
first_config_server="${CONFIG_SERVER_HOSTFILE[0]}"
second_config_server="${CONFIG_SERVER_HOSTFILE[1]}"
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
step=1
echo -e "${GREEN}Checking Shard Servers${NC}"
for shard_server in "${SHARD_SERVERS[@]}"
do
ssh "${shard_server}" /bin/bash << EOF
  sed -i -e 's|replSetName: .*|replSetName: ${SHARD_REPL_NAME}${step}|' -e 's|port: .*|port: ${SHARD_BASE_PORT}|' ${SERVER_LOCAL_PATH}/${MONGOD_SHARD_CONF_FILE}
EOF
  ((SHARD_BASE_PORT = SHARD_BASE_PORT + 1))
  ((step=step+1))
done
for shard_server in "${SHARD_SERVERS[@]}"
do
ssh "${shard_server}" /bin/bash << EOF
  mongod --config ${SERVER_LOCAL_PATH}/${MONGOD_SHARD_CONF_FILE} --fork" &
EOF
done

echo -e "${GREEN}Initializing shard replica set${NC}"
echo -e "${GREEN}Preparing shards to mongos/query router${NC}"
count=0

printf "sh.addShard(\"${SHARD_REPL_NAME}$((count+1))/" >> add_shard_to_mongos.js
printf "rs.initiate(\n{\n" > shard_replica_init.js
printf "\t_id : \"${SHARD_REPL_NAME}$((count+1))\",\n" >> shard_replica_init.js
printf "\tmembers: [\n" >> shard_replica_init.js

for shard_server in ${SHARD_SERVERS}
do
  number=0
  for ((i=0;i<"SHARD_COPY_COUNT";i++))
  do
    nodeid=$(((SHARD_SERVER_COUNT+count-i)%SHARD_SERVER_COUNT))
    current_server=`sed -n $((nodeid+1))p ${CWD}/servers | awk '{print $1}'`
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
mongos_cmd="mongos --configdb \"${CONFIG_REPL_NAME}/"
for config_server in "${CONFIG_SERVERS[@]}"
do
  mongos_cmd="${mongos_cmd}${config_server}:${MONGO_PORT},"
done
mongos_cmd=`echo ${mongos_cmd} | sed 's/,$/"/'`
mongos_cmd="${mongos_cmd} --config ${CLIENT_LOCAL_PATH}/${MONGOS_CONF_FILE} --fork"
echo $mongos_cmd
for router_server in "${ROUTER_SERVERS[@]}"
do
  ssh "${router_server}" /bin/bash << EOF
  "${mongos_cmd}" &
EOF
done

echo -e "${CYAN}waiting for router server to complete start ...${NC}"
wait
sleep 5

echo -e "${GREEN}Adding shards to mongos/query router ...${NC}"
truncate -s 0 add_shard_to_mongos.log
mongo --host ${router_server} --port ${MONGO_PORT} < add_shard_to_mongos.js >> add_shard_to_mongos.log
cat add_shard_to_mongos.log | grep -i ok

echo -e "${GREEN}Enabling sharding${NC}"
mongo --host ${router_server} --port ${MONGO_PORT} > ${MONGO_CLUSTER_PATH}/enableSharding.log << EOF
use $DATABASE_NAME;
sh.enableSharding("${DATABASE_NAME}");
db.createCollection("${COLLECTION_NAME}")
sh.shardCollection("$DATABASE_NAME.$COLLECTION_NAME",{_id:"hashed"});
db.${COLLECTION_NAME}.getShardVersion()
db.${COLLECTION_NAME}.getShardDistribution()
EOF
cat ${MONGO_CLUSTER_PATH}/enableSharding.log | grep -i ok

echo -e "${GREEN}Checking Config Servers${NC}"
for config_server in "${CONFIG_SERVER_HOSTFILE[@]}"
do
ssh "${config_server}" /bin/bash << EOF
  pgrep -la mongod
EOF
done

echo -e "${GREEN}Checking Shard Servers${NC}"
for shard_server in "${SHARD_SERVERS[@]}"
do
ssh "${shard_server}" /bin/bash << EOF
  pgrep -la mongod
EOF
done

echo -e "${GREEN}Checking Router Servers${NC}"
for router_server in "${ROUTER_SERVERS[@]}"
do
ssh "${router_server}" /bin/bash << EOF
  pgrep -la mongod
EOF
done

echo -e "${GREEN}Done starting MongoDB${NC}"
