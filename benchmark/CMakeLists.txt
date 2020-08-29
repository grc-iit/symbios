cmake_minimum_required(VERSION 3.10)
project(symbios)
set(INSTALL_DIR $ENV{INSTALL_DIR})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DH5_HAVE_DIRECT -w")
set(INCLUDE_DIRS "-I${INSTALL_DIR}/include/ -L${INSTALL_DIR}/lib/")# -DRPCLIB_ENABLE_LOGGING -DRPCLIB_ASIO=clmdep_asio -DRPCLIB_FMT=clmdep_fmt")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${INCLUDE_DIRS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${INCLUDE_DIRS}")

# Include directories
include_directories(include)
include_directories(${CMAKE_SOURCE_DIR}/common/include)

# Debug flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# Source Files
add_executable(benchmark src/benchmark.cpp include/benchmark/rng.h include/benchmark/kvs.h)
target_link_libraries(benchmark -lmpi -lhiredis -lredis++ -lmongoc-1.0 -lbson-1.0 -lbsoncxx -lboost_system -lboost_filesystem  -lmongocxx -lpthread)

#OrangeFS test cases (local)
add_custom_target(bm-usage COMMAND
        ${CMAKE_CURRENT_BINARY_DIR}/benchmark)
add_custom_target(bm-rw-test COMMAND
        ${CMAKE_CURRENT_BINARY_DIR}/benchmark
        -s orangefs -caddr ~/ -path test.bin
        -w io-only -bs 4k -fs 0k -tot 1g -wfrac .5 -rfrac .5
        -out ~/test.csv)

option(BUILD_TEST "Build the unit tests" ON)
if(BUILD_TEST)
    enable_testing()
    include(CTest)
endif()



#####ARES TEST CASES

set(SCRIPT_ROOT ${CMAKE_SOURCE_DIR}/scripts/benchmark/orange)

#Allocate test nodes
add_custom_target(ares-alloc-compute
        COMMAND salloc --reservation=llogan_6 -p compute -w ares-comp-[29-30] --exclusive)
add_custom_target(ares-alloc-storage
        COMMAND salloc --reservation=llogan_6 -p storage -w ares-stor-[01-08] --exclusive)

#Deploy OrangeFS
add_custom_target(ares-deploy-hdd-4
        COMMAND ${SCRIPT_ROOT}/deploy.sh
            ${SCRIPT_ROOT}/conf/small/basic.conf
            /mnt/hdd/llogan/orangefs
            /mnt/nvme/llogan/write/generic
            ${SCRIPT_ROOT}/hostfiles/hostfile_servers_small
            ${SCRIPT_ROOT}/hostfiles/hostfile_clients)

#Terminate OrangeFS
add_custom_target(ares-terminate-hdd-4
        COMMAND ${SCRIPT_ROOT}/deploy.sh
        ${SCRIPT_ROOT}/orange/conf/small/basic.conf
        /mnt/hdd/llogan/orangefs
        /mnt/nvme/llogan/write/generic
        ${SCRIPT_ROOT}/hostfiles/hostfile_servers_small
        ${SCRIPT_ROOT}/hostfiles/hostfile_clients)

#OrangeFS test cases (ares)
add_custom_target(ares_bm_s_orangefs_bs_4k_fs_0k_tot_1g_wfrac_1_hdd_4 COMMAND
        mpirun -n 1 --host=${SCRIPT_ROOT}/hostfiles/hostfile_clients ${CMAKE_CURRENT_BINARY_DIR}/benchmark
        -s orangefs -caddr /mnt/nvme/llogan/write/generic -path /test.bin
        -w io-only -bs 4k -fs 0k -tot 1g -wfrac 1 -rfrac 0
        -out ~/test_hdd4_basic)








