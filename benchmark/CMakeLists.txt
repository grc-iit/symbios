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

#Allocate test nodes (Ares)
add_custom_target(ares-alloc-client
        COMMAND salloc --reservation=llogan_6 -w ${CMAKE_SOURCE_DIR}/scripts/hostfiles/hostfile_clients)
add_custom_target(ares-alloc-hdd-4
        COMMAND salloc --reservation=llogan_6 -w ${CMAKE_SOURCE_DIR}/scripts/hostfiles/hostfile_servers_small)
add_custom_target(ares-alloc-hdd-24
        COMMAND salloc --reservation=llogan_6 -w ${CMAKE_SOURCE_DIR}/scripts/hostfiles/hostfile_servers_big)
add_custom_target(ares-alloc-ssd-4
        COMMAND salloc --reservation=llogan_6 -w ${CMAKE_SOURCE_DIR}/scripts/hostfiles/hostfile_servers_ssd)
add_custom_target(ares-alloc-nvme-4
        COMMAND salloc --reservation=llogan_6 -w ${CMAKE_SOURCE_DIR}/scripts/hostfiles/hostfile_servers_nvme)

#Deploy OrangeFS

#OrangeFS test cases (ares)
add_custom_target(ares_bm_bs-4k_fs-0k_tot-1g_wfrac-1_hdd-4 COMMAND
        mpirun -n 1 --host=${CMAKE_SOURCE_DIR}/scripts/hostfile_clients ${CMAKE_CURRENT_BINARY_DIR}/benchmark
        -s orangefs -caddr /mnt/hdd/llogan/orangefs/data -path test.bin
        -w io-only -bs 4k -fs 0k -tot 1g -wfrac 1 -rfrac 0
        -out ~/test.csv)








