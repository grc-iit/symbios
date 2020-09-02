#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

CWD="$(pwd)"
HOSTFILE=@1
REDIS_DIR=@2

SERVERS="($(cat "${HOSTFILE}"))"

echo -e "${GREEN}Stopping Redis ...${NC}"
for server in "${SERVERS[@]}"
do
ssh "${server}" /bin/bash << EOF
  killall redis-server > /dev/null
  rm -rf "${REDIS_DIR}"/*
EOF
done
echo -e "${GREEN}Redis is stopped${NC}"
