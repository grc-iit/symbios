#!/bin/bash

#Remeber to have env Variables for:
# ORANGEFS_KO
# ORANGEFS_PATH
# PVFS2TAB_FILE

#Input Variables
conf_file=${1}
server_dir=${2}
client_dir=${3}
server_hostfile=${4}
client_hostfile=${5}

#General Variables
client_list=($(cat ${client_hostfile}))
server_list=($(cat ${server_hostfile}))

#Config PFS
name="orangefs" #TODO: Allow renaming
comm_port=3334  #TODO: Allow changing

#Change the user in the config files
sed -i "s/USER/${USER}/g" ${conf_file}

#edit the pvfs2tab
echo "tcp://${server_list[0]}:${comm_port}/${name} ${client_dir} pvfs2 defaults,noauto 0 0" > $PVFS2TAB_FILE

#Server Setup
for node in ${server_list[@]}
do
ssh ${node} /bin/bash << EOF
echo "Setting up server at ${node} "
source ~/.bashrc
rm -rf ${server_dir}*
mkdir -p ${server_dir}
pvfs2-server -f -a ${node} ${conf_file}
pvfs2-server -a ${node} ${conf_file}
EOF
done

#Client Setup
for node in ${client_list[@]}
do
ssh ${node} /bin/bash << EOF
echo "Starting client on ${node}"
source ~/.bashrc
sudo kill-pvfs2-client
mkdir -p ${client_dir} 
sudo insmod ${ORANGEFS_KO}/pvfs2.ko
sudo ${ORANGEFS_PATH}/sbin/pvfs2-client -p ${ORANGEFS_PATH}/sbin/pvfs2-client-core
sudo mount -t pvfs2 tcp://${server_list[0]}:${comm_port}/${name} ${client_dir}
EOF
done
