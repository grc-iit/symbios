#!/bin/bash

RUN_ORANGE="${HOME}/symbios/scripts/distributed/OrangeFS/terminate.sh"
RUN_REDIS="${HOME}/symbios/scripts/distributed/Redis/terminate.sh"
RUN_MONGO="${HOME}/symbios/scripts/distributed/MongoDB/terminate.sh"

#ORANGE_FS
conf_file="${HOME}/symbios/scripts/distributed/OrangeFS/conf/iris.config"
server_dir="/opt/ohpc/pub/orangefs/storage/data" #If you are going to touch this one, you need to change the config files
client_dir="${HOME}/symbios_data/OrangeFS/client"
server_hostfile="${HOME}/symbios/scripts/distributed/hostfile/orange_server_iris"
client_hostfile="${HOME}/symbios/scripts/distributed/hostfile/client"

#REDIS
HOSTFILE="${HOME}/symbios/scripts/distributed/hostfile/redis_server_iris"
LOG_DIR="${HOME}/symbios_data/Redis"

#MONGO
SERVER_HOSTFILE="${HOME}/symbios/scripts/distributed/hostfile/mongo_server_iris"

${RUN_ORANGE} "${conf_file}" "${server_dir}" "${client_dir}" "${server_hostfile}" "${client_hostfile}"
${RUN_REDIS} "${HOSTFILE}" "${LOG_DIR}"
${RUN_MONGO} "${SERVER_HOSTFILE}" "${client_hostfile}"