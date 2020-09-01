#!/bin/bash

#Remeber to have env Variables for:
# ORANGEFS_KO
# ORANGEFS_PATH
# PVFS2TAB_FILE

CWD=$(pwd)

#Input Variables
conf_file=${1}
server_dir=${2}
client_dir=${3}
server_hostfile=${4}

#General Variables
client_list=($(cat ${CWD}/hostfiles/hostfile_clients))
server_list=($(cat ${server_hostfile}))

#Config PFS
name="orangefs" #TODO: Allow renaming
comm_port=3334  #TODO: Allow changing

#Stop clients
for node in ${client_list[@]}
do
ssh ${node} /bin/bash << EOF
echo "Stopping client on $node"
sudo /usr/sbin/kill-pvfs2-client
EOF
done

#Stop servers
for node in ${server_list[@]}
do
ssh ${node} /bin/bash << EOF
echo "Killing server at ${node} "
sudo /usr/sbin/kill-pvfs2-client
rm -rf ${server_dir}/*
killall -s SIGKILL pvfs2-server
EOF
done

echo "done"
