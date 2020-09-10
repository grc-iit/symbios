#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

CWD="$(pwd)"
HOSTFILE=${1}
MASTER_SETUP_FOLDER=${2} #${HOME}/symbios-setup/Redis
LOG_DIR="/mnt/hdd/${USER}/Redis"

PORT_BASE=7000
SERVERS=($(cat ${HOSTFILE}))
echo -e "${CYAN}${SERVERS[*]}${NC}"

# Prepare configuration for each server
echo -e "${GREEN}Preparing Redis cluster configuration files${NC}"
i=0
for server in "${SERVERS[@]}"; do
  server_ip=$(getent ahosts "${server}" | grep STREAM | awk '{print $1}')
  ((port = $PORT_BASE + $i))
  mkdir -p "${MASTER_SETUP_FOLDER}"/"${port}"
  rm -rf "${MASTER_SETUP_FOLDER}"/"${port}"/redis.conf
  (
    echo "port $port"
    echo "cluster-enabled yes"
    echo "cluster-config-file ${LOG_DIR}/${port}/nodes.conf"
    echo "cluster-node-timeout 5000"
    echo "appendonly yes"
    echo "protected-mode no"
    echo "logfile ${LOG_DIR}/${port}/file.log"
    echo "dir ${LOG_DIR}"
  ) >>"${MASTER_SETUP_FOLDER}"/"${port}"/redis.conf
  ((i = i + 1))
done

# Copy configuration files to local directories on all servers
echo -e "${GREEN}Copying Redis cluster configuration files ...${NC}"
i=0
for server in "${SERVERS[@]}"; do
  ((port = $PORT_BASE + $i))
  echo Copying configuration directory $port to $server ...
  ssh $server mkdir -p "${LOG_DIR}"
  rsync -qraz "${MASTER_SETUP_FOLDER}"/"${port}" $server:"${LOG_DIR}"/ &
  ((i = i + 1))
done
wait

# Start server
echo -e "${GREEN}Starting Redis${NC}"
i=0
for server in "${SERVERS[@]}"; do
  ((port = $PORT_BASE + $i))
  echo "Starting redis on ${server}:${port}"
  ssh "${server}" /bin/bash <<EOF
  redis-server ${LOG_DIR}/$port/redis.conf > /dev/null 2>&1 &
  echo -e "${GREEN}Verifying Redis cluster${NC}"
  pgrep -l redis-server
EOF
  ((i = i + 1))
done
wait

# Connect servers
echo -e "${GREEN}Connecting Redis cluster servers${NC}"
cmd="redis-cli --cluster create "
i=0
for server in "${SERVERS[@]}"; do
  server_ip=$(getent ahosts "${server}" | grep STREAM | awk '{print $1}')
  ((port = $PORT_BASE + $i))
  cmd="${cmd}${server_ip}:${port} "
  ((i = i + 1))
done
cmd="${cmd}--cluster-replicas 0"
echo "Cluster command: ${cmd}"
echo yes | $cmd

# Check cluster nodes
echo -e "${GREEN}Checking Redis cluster nodes${NC}"
first_server="${SERVERS[0]}"
cmd="redis-cli -c -h ${first_server} -p ${PORT_BASE} cluster nodes"
$cmd | sort -k9 -n
echo -e "${GREEN}Redis is started${NC}"
