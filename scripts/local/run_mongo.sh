#!/bin/bash
if [ $# -lt 2 ]
then
  echo "Usage: ./run_mongo.sh mongo_config_path mongo_server_install_path clean_flag"
  echo "e.g ./run_mongo.sh ~/mongo_config_path ~/mongo_server_install_path [true/false]"
  exit
fi

#Input Variables
MONGO_CLUSTER_DIR=${1}
MONGO_INSTALL_DIR=${2}
CLEAN_FLAG=${3:-true}

CWD=$(pwd)

MONGO_SERVER_BIN=${MONGO_INSTALL_DIR}/bin/mongod
MONGO_CLIENT_BIN=${MONGO_INSTALL_DIR}/bin/mongo

CONFIG_FILE=mongodb.conf
PORT_BASE=27017

if [ x"${MONGO_INSTALL_DIR}" = x ]
then
  echo "You must set a mongo install directory, exiting ..."
  exit
fi

if [ x"${MONGO_CLUSTER_DIR}" = x ]
then
  echo "You must set a mongo config directory, exiting ..."
  exit
fi


# clean mongo cluster directory
if [ ${CLEAN_FLAG} = "true" ]
then
  rm -rf  ${MONGO_CLUSTER_DIR}/*
fi

#create db and logs directory for mongodb
mkdir -p ${MONGO_CLUSTER_DIR}/db
mkdir -p ${MONGO_CLUSTER_DIR}/logs
mkdir -p ${MONGO_CLUSTER_DIR}/conf

#create mongodb configuration file
echo "dbpath=${MONGO_CLUSTER_DIR}/db" > ${MONGO_CLUSTER_DIR}/conf/${CONFIG_FILE}
echo "logpath=${MONGO_CLUSTER_DIR}/logs/mongodb.log" >> ${MONGO_CLUSTER_DIR}/conf/${CONFIG_FILE}
echo "port=$PORT_BASE" >> ${MONGO_CLUSTER_DIR}/conf/${CONFIG_FILE}
echo "fork=true" >> ${MONGO_CLUSTER_DIR}/conf/${CONFIG_FILE}

# Start the mongo server
${MONGO_SERVER_BIN} -f ${MONGO_CLUSTER_DIR}/conf/${CONFIG_FILE} > /dev/null 2>&1 &

# sleep 5 seconds to wait mongo server start ok
sleep 5

# Verify mongo server
echo "Verifying mongo server...."
nums=`pgrep -l mongod | wc -l`
if [[ $nums -lt 1 ]]
then
  echo "There may have someting wrong when starting mongo server, exiting..."
  exit
fi

echo "Mongo server is started"
