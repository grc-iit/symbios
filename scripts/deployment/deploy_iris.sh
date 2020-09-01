#!/bin/bash

${HOME}/symbios/scripts/local/run_redis_cluster.sh
${HOME}/symbios/scripts/local/run_orangefs.sh
${HOME}/symbios/scripts/local/run_mongo.sh

#conf_file=${1}
#server_dir=${2}
#client_dir=${3}
#server_hostfile=${4}

#REDIS_CLUSTER_DIR=${1}
#REDIS_SERVER_NUMS=${2}
#REDIS_INSTALL_DIR=${3}
#CLEAN_FLAG=${4:-true}

#MONGO_SERVER_BIN=${MONGO_INSTALL_DIR}/bin/mongod
#MONGO_CLIENT_BIN=${MONGO_INSTALL_DIR}/bin/mongo

#Use 24 node case. for irtis and niobe use 8 nodes per solutions for symbios use 24 node for each and colocate.