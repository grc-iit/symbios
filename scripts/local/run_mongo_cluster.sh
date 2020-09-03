#!/bin/bash
if [ $# -lt 5 ]
then
    echo "Usage: ./run_mongo_cluster.sh mongo_cluster_install_path shard_server_nums mongo_server_install_path database_name collection_name clean_flag"
    echo "e.g ./run_mongo_cluster.sh ~/mongo_cluster_install_path shard_server_nums ~/mongo_server_install_path database_name collection_name [true/false]"
    exit
fi

MONGO_CLUSTER_PATH=${1}
SHARD_SERVER_NUMS=${2}
MONGO_SERVER_INTSALL_PATH=${3}
DATABASE_NAME=${4}
COLLECTION_NAME=${5}
CLEAN_FLAG=${6:-true}

if [ x"${MONGO_CLUSTER_PATH}" = x ]
then
  echo "You must set a mongo cluster install directory, exiting ..."
  exit
fi

if [[ ${SHARD_SERVER_NUMS} -lt 1 ]]
then
  echo "At least 1 shard server for mongo cluster, exiting ..."
  exit
fi

if [ x"${MONGO_SERVER_INTSALL_PATH}" = x ]
then
  echo "You must set the mongo server install path,otherwise we cannot find the mongod/mongos/mongo command. exiting ..."
  exit
fi

if [ x"${DATABASE_NAME}" = x ]
then
  echo "You must give the database_name which is needed to shard. exiting ..."
  exit
fi

if [ x"${COLLECTION_NAME}" = x ]
then
  echo "You must give the collection_name which is needed to shard. exiting ..."
  exit
fi

MONGO_SERVER_BIN=${MONGO_SERVER_INTSALL_PATH}/bin/mongod
MONGOS_BIN=${MONGO_SERVER_INTSALL_PATH}/bin/mongos
MONGO_CLIENT_BIN=${MONGO_SERVER_INTSALL_PATH}/bin/mongo

# congigure and start config server
CONFIG_SERVER_CONF_FILE="mongodb_config.conf"
if [ ${CLEAN_FLAG} = "true" ]
then
  rm -rf ${MONGO_CLUSTER_PATH}/config_server/*
fi
mkdir -p ${MONGO_CLUSTER_PATH}/config_server/db
mkdir -p ${MONGO_CLUSTER_PATH}/config_server/conf
mkdir -p ${MONGO_CLUSTER_PATH}/config_server/logs
CONFIG_SERVER_BASE_PORT=57040
# create config server configuration
echo "dbpath=${MONGO_CLUSTER_PATH}/config_server/db" > ${MONGO_CLUSTER_PATH}/config_server/conf/${CONFIG_SERVER_CONF_FILE}
echo "logpath=${MONGO_CLUSTER_PATH}/config_server/logs/cfg.log" >> ${MONGO_CLUSTER_PATH}/config_server/conf/${CONFIG_SERVER_CONF_FILE}
echo "port=${CONFIG_SERVER_BASE_PORT}" >> ${MONGO_CLUSTER_PATH}/config_server/conf/${CONFIG_SERVER_CONF_FILE}
echo "fork=true" >> ${MONGO_CLUSTER_PATH}/config_server/conf/${CONFIG_SERVER_CONF_FILE}
# start config server
${MONGO_SERVER_BIN} --replSet configset -f ${MONGO_CLUSTER_PATH}/config_server/conf/${CONFIG_SERVER_CONF_FILE} --configsvr

#initialize config server
config_server_addr="localhost:${CONFIG_SERVER_BASE_PORT}"
${MONGO_CLIENT_BIN} --port ${CONFIG_SERVER_BASE_PORT} << EOF
  config = {_id: "configset", members:[{_id : 0, host : "${config_server_addr}"}]}
  rs.initiate(config)
EOF

SHARD_SERVER_CONF_FILE=mongodb_shard.conf
SHARD_SERVER_BASE_PORT=37017
# start sharding servers
if [ ${CLEAN_FLAG} = "true" ]
then
  rm -rf ${MONGO_CLUSTER_PATH}/shard*
fi
for seq in `seq 1 ${SHARD_SERVER_NUMS}`
do
  ((index=$seq-1))
  ((shard_server_port=$SHARD_SERVER_BASE_PORT+$index))
  SHARD_DIR=${MONGO_CLUSTER_PATH}/shard${index}
  mkdir -p ${SHARD_DIR}/rs0
  mkdir -p ${SHARD_DIR}/conf
  mkdir -p ${SHARD_DIR}/logs

  #create configuration for sharding servers
  echo "dbpath=${SHARD_DIR}/rs0" > ${SHARD_DIR}/conf/${SHARD_SERVER_CONF_FILE}
  echo "logpath=${SHARD_DIR}/logs/rs0_${shard_server_port}.log" >> ${SHARD_DIR}/conf/${SHARD_SERVER_CONF_FILE}
  echo "port=${shard_server_port}" >> ${SHARD_DIR}/conf/${SHARD_SERVER_CONF_FILE}
  echo "fork=true" >> ${SHARD_DIR}/conf/${SHARD_SERVER_CONF_FILE}

  ${MONGO_SERVER_BIN} --replSet set${index} -f ${SHARD_DIR}/conf/${SHARD_SERVER_CONF_FILE} --shardsvr --oplogSize 50 --timeStampFormat "iso8601-local"
done
sleep 5

# Intialize Sharding server
for seq in `seq 1 ${SHARD_SERVER_NUMS}`
do
  ((index=$seq-1))
  ((shard_server_port=$SHARD_SERVER_BASE_PORT+$index))
  repl_name="set${index}"
  repl_addr="localhost:${shard_server_port}"
  ${MONGO_CLIENT_BIN} --port ${shard_server_port} << EOF
  config = {_id: "${repl_name}", members:[{_id : 0, host : "${repl_addr}"}]}
  rs.initiate(config)
EOF
done

#start and initialize the router
ROUTER_SERVER_CONF_FILE="mongo_router.conf"
ROUTER_SERVER_BASE_PORT=27017

if [ ${CLEAN_FLAG} = "true" ]
then
  rm -rf ${MONGO_CLUSTER_PATH}/router/*
fi
mkdir -p ${MONGO_CLUSTER_PATH}/router/logs

${MONGOS_BIN} --logpath "${MONGO_CLUSTER_PATH}/router/logs/mongos.log" --configdb "configset/${config_server_addr}" --port ${ROUTER_SERVER_BASE_PORT} --fork  --timeStampFormat "iso8601-local"
#add shard servers to the router
for seq in `seq 1 ${SHARD_SERVER_NUMS}`
do
  ((index=$seq-1))
  ((shard_server_port=$SHARD_SERVER_BASE_PORT+$index))
  shard_content="set${index}/localhost:${shard_server_port}"
  ${MONGO_CLIENT_BIN} --port ${ROUTER_SERVER_BASE_PORT} << EOF
  db.adminCommand( { addshard : "${shard_content}" } );
EOF
done

# Enable the sharding
echo "Database ${DATABASE_NAME} -> sharding collection ${COLLECTION_NAME}..."
#sharding the collection
${MONGO_CLIENT_BIN} --port ${ROUTER_SERVER_BASE_PORT} > ${MONGO_CLUSTER_PATH}/enableSharding.log << EOF
use $DATABASE_NAME;
sh.enableSharding("${DATABASE_NAME}");
db.createCollection("${COLLECTION_NAME}")
sh.shardCollection("$DATABASE_NAME.$COLLECTION_NAME",{_id:"hashed"});
db.${COLLECTION_NAME}.getShardVersion()
db.${COLLECTION_NAME}.getShardDistribution()
EOF
cat ${MONGO_CLUSTER_PATH}/enableSharding.log | grep -i ok

echo "Mongo cluster started..."