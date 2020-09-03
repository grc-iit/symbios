#!/bin/bash
#!/bin/bash
if [ $# -lt 3 ]
then
    echo "Usage: ./checking_mongo_shard.sh mongo_cluster_install_path mongo_server_install_path database_name collection_name"
    exit
fi

MONGO_CLUSTER_PATH=${1}
MONGO_SERVER_INTSALL_PATH=${2}
DATABASE_NAME=${3}
COLLECTION_NAME=${4}

if [ x"${MONGO_CLUSTER_PATH}" = x ]
then
  echo "You must set a mongo cluster install directory, exiting ..."
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

echo "Database ${DATABASE_NAME} -> sharding collection ${COLLECTION_NAME}..."
MONGO_CLIENT_BIN=${MONGO_SERVER_INTSALL_PATH}/bin/mongo

ROUTER_BASE_PORT=27017
#sharding the collection
${MONGO_CLIENT_BIN} --port 27017 > ${MONGO_CLUSTER_PATH}/checkSharding.log << EOF
use $1;
db.${2}.getShardVersion()
db.${2}.getShardDistribution()
EOF
echo "Finish sharding checking, please check the log '${MONGO_CLUSTER_PATH}/checkSharding.log'"
