#!/bin/bash
if [ $# -lt 3 ]
then
  echo "Usage: ./run_redis_cluster.sh redis_cluster_config_path redis_server_numbers redis_install_path clean_flag"
  echo "e.g ./run_redis_cluster.sh ~/redis_config 3 ~/redis_install [true/false]"
  exit
fi


#Input Variables
REDIS_CLUSTER_DIR=${1}
REDIS_SERVER_NUMS=${2}
REDIS_INSTALL_DIR=${3}
CLEAN_FLAG=${4:-true}

CWD=$(pwd)

REDIS_SERVER_BIN=${REDIS_INSTALL_DIR}/bin/redis-server
REDIS_CLIENT_BIN=${REDIS_INSTALL_DIR}/bin/redis-cli

# download redis source and compile
if [ ! -f "$REDIS_SERVER_BIN" -a ! -f "$REDIS_CLIENT_BIN" ]
then
  echo "Downloading Redis source package....."
#  wget https://github.com/redis/redis/archive/6.0.6.tar.gz
#  tar -xzf 6.0.6.tar.gz
#  cd redis-6.0.6
#  make PREFIX=${REDIS_INSTALL_DIR}
#  cd -
#  echo "Finish compiling Redis source...."
fi


CONFIG_FILE=redis.conf
PORT_BASE=6379

if [ x"${REDIS_INSTALL_DIR}" = x ]
then
  echo "You must set a redis cluster install directory, exiting ..."
  exit
fi

if [ x"${REDIS_CLUSTER_DIR}" = x ]
then
  echo "You must set a redis cluster config directory, exiting ..."
  exit
fi

if [[ $REDIS_SERVER_NUMS -lt 3 ]]
then
  echo "At least 3 servers are required for redis cluster, exiting ..."
  exit
fi

if [ ${CLEAN_FLAG} = "true" ]
then
  # clean redis cluster directory
  rm -rf  ${REDIS_CLUSTER_DIR}/*
fi

# create Redis server list
for seq in `seq 1 ${REDIS_SERVER_NUMS}`
do
  # create redis server
  redis_server_name="Redis_"${seq}
  redis_server_dir=${REDIS_CLUSTER_DIR}/${redis_server_name}
  mkdir -p ${redis_server_dir}
  ((index=$seq-1))
  ((redis_port=$PORT_BASE+$index))
  echo "redis_server_name: "${redis_server_name}
  echo "redis_port: "${redis_port}
  # copy redis_server_bin & redis_client_bin to redis_server_directory
  cp $REDIS_SERVER_BIN  ${redis_server_dir}
  cp $REDIS_CLIENT_BIN  ${redis_server_dir}
  # create the conf file
  echo "port $redis_port" > ${redis_server_dir}/$CONFIG_FILE
  echo "protected-mode no" >> ${redis_server_dir}/$CONFIG_FILE
  echo "daemonize yes" >> ${redis_server_dir}/$CONFIG_FILE
  echo "pidfile ${redis_server_dir}/redis.pid" >> ${redis_server_dir}/$CONFIG_FILE
  echo "cluster-enabled yes" >> ${redis_server_dir}/$CONFIG_FILE
  echo "cluster-config-file nodes.conf" >>  ${redis_server_dir}/$CONFIG_FILE
  echo "cluster-node-timeout 5000" >> ${redis_server_dir}/$CONFIG_FILE
  echo "appendonly yes" >> ${redis_server_dir}/$CONFIG_FILE
  echo "dir ${redis_server_dir}" >> ${redis_server_dir}/$CONFIG_FILE
  echo "logfile ${redis_server_dir}/file.log" >> ${redis_server_dir}/$CONFIG_FILE
done

# Start the Redis Server respectively
for seq in `seq 1 ${REDIS_SERVER_NUMS}`
do
  redis_server_name="Redis_"${seq}
  redis_server_dir=${REDIS_CLUSTER_DIR}/${redis_server_name}
  # start the server
   ${REDIS_SERVER_BIN} ${redis_server_dir}/$CONFIG_FILE > /dev/null 2>&1 &
done
# sleep 5 seconds to wait all the redis-server start ok
sleep 5

# Verify Redis server
echo "Verifying Redis Cluster servers...."
nums=`pgrep -l redis-server | wc -l`
if [[ $nums -lt $REDIS_SERVER_NUMS ]]
then
  echo "Requested redis server numbers are "${REDIS_SERVER_NUMS}", however, started redis server numbers are "${nums}
  echo "There may have someting wrong when starting Redis servers, exiting..."
  exit
fi
# creating redis cluster by redis-cli
cmd="$REDIS_CLIENT_BIN --cluster create "
for seq in `seq 1 ${REDIS_SERVER_NUMS}`
do
  ((index=$seq-1))
  ((redis_port=$PORT_BASE+$index))
  cmd="${cmd}127.0.0.1:${redis_port} "
done

cmd="${cmd}--cluster-replicas 0"
echo yes | $cmd

sleep 2

# check cluster status
echo "Checking Redis Cluster status......"
cmd="$REDIS_CLIENT_BIN -p ${PORT_BASE} cluster nodes"
$cmd | sort -k9 -n

echo "Redis cluster is started"
