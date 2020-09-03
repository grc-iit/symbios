#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

CWD="$(pwd)"
HOSTFILE=${1}
LOG_DIR=${2}

PORT_BASE=7000
SERVERS=($(cat ${HOSTFILE}))

# Prepare configuration for each server
echo -e "${GREEN}Preparing Redis cluster configuration files ...${NC}"
i=0
for server in "${SERVERS[@]}"; do
  server_ip=$(getent ahosts "${server}" | grep STREAM | awk '{print $1}')
  ((port = $PORT_BASE + $i))
  mkdir -p "${LOG_DIR}"/"${port}"
  rm -rf "${LOG_DIR}"/"${port}"/redis.conf
  (
    echo "port $port"
    echo "cluster-enabled yes"
    echo "cluster-config-file nodes.conf"
    echo "cluster-node-timeout 5000"
    echo "appendonly yes"
    echo "protected-mode no"
    echo "logfile ${LOG_DIR}/${port}/file.log"
  ) >>"${LOG_DIR}"/"${port}"/redis.conf
  ((i = i + 1))
done

# Start server
echo -e "${GREEN}Starting Redis${NC}"
i=0
for server in "${SERVERS[@]}"; do
  ((port = $PORT_BASE + $i))
  echo "Starting redis on ${server}:${port}"
  ssh "${server}" /bin/bash <<EOF
  cd ${LOG_DIR}/$port
  redis-server ./redis.conf > /dev/null 2>&1 &
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
  server_ip=$(getent ahosts "${server}${HOSTNAME_POSTFIX}" | grep STREAM | awk '{print $1}')
  ((port = $PORT_BASE + $i))
  cmd="${cmd}${server_ip}:${port} "
  ((i = i + 1))
done
cmd="${cmd}--cluster-replicas 1"
echo yes | $cmd

# Check cluster nodes
echo -e "${GREEN}Checking Redis cluster nodes${NC}"
first_server="${SERVERS[0]}"
cmd="redis-cli -c -h ${first_server}${HOSTNAME_POSTFIX} -p ${PORT_BASE} cluster nodes"
$cmd | sort -k9 -n
echo -e "${GREEN}Redis is started${NC}"
