#!/bin/bash

SOURCE_DIR=${HOME}/symbios

CONFIG_PATH="${SOURCE_DIR}/conf/test.conf"
HOSTFILE_PATH="${SOURCE_DIR}/conf/server_lists"
LOG_PATH="${SOURCE_DIR}/test/server.lock"
SERVER_PATH="${SOURCE_DIR}/build/symbios_server"
CLIENT_PATH="${SOURCE_DIR}/build/test/unit/unit_client"

SERVER_PROCESSES=(1 2)
CLIENT_PROCESSES=(1 2)
IO_MODE=(0) #0 is file, 1 is redis, 2 is mongo
IO_OPERATION=(0 1 2)
REQUEST_SIZE=(5 10)
REQUEST_NUMBER=(1 2 4)
DISTRIBUTION_MODE=("RANDOM_POLICY" "ROUND_ROBIN_POLICY") #HEURISTICS_POLICY DYNAMIC_PROGRAMMING_POLICY
TEST_COUNT=0

for n_server_processes in "${SERVER_PROCESSES[@]}"
do
  for n_client_processes in "${CLIENT_PROCESSES[@]}"
  do
    for io_mode in "${IO_MODE[@]}"
    do
      for io_operation in "${IO_OPERATION[@]}"
      do
        for distribution_mode in "${DISTRIBUTION_MODE[@]}"
        do
          for request_size in "${REQUEST_SIZE[@]}"
          do
            for request_number in "${REQUEST_NUMBER[@]}"
            do
              echo "Test ${TEST_COUNT}: IO_MODE: ${io_mode}, IO_OPERATION: ${io_operation}, REQUEST_SIZE: ${request_size}, REQUEST_NUMBER: ${request_number}, DISTRIBUTION_MODE: ${distribution_mode}"

              echo "localhost:${n_client_processes}" > "${HOSTFILE_PATH}"/single_node_symbios_client
              echo "localhost:${n_server_processes}" > "${HOSTFILE_PATH}"/single_node_symbios_server

              echo "Starting Server"
              echo "${SERVER_PATH} ${CONFIG_PATH}"
              nohup mpirun -n "${n_server_processes}" --hostfile "${HOSTFILE_PATH}"/single_node_symbios_server "${SERVER_PATH}" "${CONFIG_PATH}" &
              echo "sleeping for 5 seconds"
              sleep 5

              echo "Starting Client"
              echo "mpirun -n ${n_client_processes} --hostfile ${HOSTFILE_PATH}/single_node_symbios_client ./test/unit/unit_client ${CONFIG_PATH} ${io_mode} ${io_operation} ${distribution_mode} ${request_size} ${request_number}"
              mpirun -n "${n_client_processes}" --hostfile "${HOSTFILE_PATH}"/single_node_symbios_client "${CLIENT_PATH}" "${CONFIG_PATH}" "${io_mode}" "${io_operation}" "${distribution_mode}" "${request_size}" "${request_number}"

              echo "Stopping Server"
              killall mpirun
              ((TEST_COUNT=TEST_COUNT+1))
            done
          done
        done
      done
    done
  done
done