#!/bin/bash

CWD=$(pwd)
SEARCH_DIR=$CWD/conf/

client_dir=/mnt/nvme/${USER}/write/generic

# HDD 4 nodes
for entry in "$SEARCH_DIR"/small/*
do
  ${CWD}/deploy.sh ${entry} /mnt/hdd/${USER}/orangefs ${client_dir} ${CWD}/hostfiles/hostfile_servers_small



  ${CWD}/terminate.sh ${entry} /mnt/hdd/${USER}/orangefs ${client_dir} ${CWD}/hostfiles/hostfile_servers_small
done

#HDD 24 nodes
for entry in "$SEARCH_DIR"/big/*
do
  ${CWD}/deploy.sh ${entry} /mnt/hdd/${USER}/orangefs ${client_dir} ${CWD}/hostfiles/hostfile_servers_big



  ${CWD}/terminate.sh ${entry} /mnt/hdd/${USER}/orangefs ${client_dir} ${CWD}/hostfiles/hostfile_servers_big
done

#SSD 4 nodes
for entry in "$SEARCH_DIR"/ssd/*
do
  ${CWD}/deploy.sh ${entry} /mnt/ssd/${USER}/orangefs ${client_dir} ${CWD}/hostfiles/hostfile_servers_ssd



  ${CWD}/terminate.sh ${entry} /mnt/ssd/${USER}/orangefs ${client_dir} ${CWD}/hostfiles/hostfile_servers_ssd
done

#NVME 4 nodes
for entry in "$SEARCH_DIR"/nvme/*
do
  ${CWD}/deploy.sh ${entry} /mnt/nvme/${USER}/orangefs ${client_dir} ${CWD}/hostfiles/hostfile_servers_nvme



  ${CWD}/terminate.sh ${entry} /mnt/nvme/${USER}/orangefs ${client_dir} ${CWD}/hostfiles/hostfile_servers_nvme
done
