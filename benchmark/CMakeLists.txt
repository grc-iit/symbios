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
include_directories(src)
include_directories(${CMAKE_SOURCE_DIR}/common/include)

# Debug flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# Source Files
add_executable(benchmark src/benchmark/benchmark.cpp ${CMAKE_SOURCE_DIR}/common/include/common/rng.h src/benchmark/kvs.h src/benchmark/mongo.h src/benchmark/orangefs.h  src/benchmark/redis.h ../common/include/common/rng.h)
target_link_libraries(benchmark -lmpi -lhiredis -lredis++ -lmongoc-1.0 -lbson-1.0 -lbsoncxx -lboost_system -lboost_filesystem  -lmongocxx -lpthread)

# Enable tests
option(BUILD_TEST "Build the unit tests" ON)
if(BUILD_TEST)
    enable_testing()
    include(CTest)
endif()

#####LOCAL TEST CASES

add_test(NAME bm_usage COMMAND
        ${CMAKE_CURRENT_BINARY_DIR}/benchmark)
add_test(NAME bm_prealloc COMMAND
        ${CMAKE_CURRENT_BINARY_DIR}/benchmark
        -s orangefs -caddr /home/lukemartinlogan/ -path test.bin
        -w prealloc -fs 1b)
add_test(NAME bm_rw_test_seq COMMAND
        ${CMAKE_CURRENT_BINARY_DIR}/benchmark
        -s orangefs -caddr /home/lukemartinlogan/ -path test.bin
        -w io-only -bs 4k -fs 0k -tot 1g -wfrac .5 -rfrac .5 -ap uniform -seed 2542
        -out ~/io_test.csv)
add_test(NAME bm_rw_test_rand COMMAND
        ${CMAKE_CURRENT_BINARY_DIR}/benchmark
        -s orangefs -caddr /home/lukemartinlogan/ -path test.bin
        -w io-only -bs 4k -fs 0k -tot 1g -wfrac .5 -rfrac .5 -ap uniform -seed 2542
        -out ~/io_test.csv)
add_test(NAME bm_md_test COMMAND
        ${CMAKE_CURRENT_BINARY_DIR}/benchmark
        -s orangefs -caddr /home/lukemartinlogan/ -path /test
        -w md-fs -md_depth 1 -md_fcnt 16k
        -out ~/md_test.csv)


#####ARES TEST CASES

set(SCRIPT_ROOT ${CMAKE_SOURCE_DIR}/scripts/benchmark/orange)

#Allocate test nodes
add_custom_target(ares_alloc_compute
        COMMAND salloc --reservation=llogan_6 -p compute -w ares-comp-[29-30] --exclusive)
add_custom_target(ares_alloc_storage
        COMMAND salloc --reservation=llogan_6 -p storage -w ares-stor-[01-08] --exclusive)

#Deploy OrangeFS (example)
add_custom_target(ares_deploy_orangefs_hdd_4_basic
        COMMAND ${SCRIPT_ROOT}/deploy.sh
            ${SCRIPT_ROOT}/conf/small/basic.conf
            /mnt/hdd/llogan/orangefs
            /mnt/nvme/llogan/write/generic
            ${SCRIPT_ROOT}/hostfiles/hostfile_servers_small
            ${SCRIPT_ROOT}/hostfiles/hostfile_clients)

#Terminate OrangeFS (example)
add_custom_target(ares_terminate_hdd_4_basic
        COMMAND ${SCRIPT_ROOT}/terminate.sh
        ${SCRIPT_ROOT}/conf/small/basic.conf
        /mnt/hdd/llogan/orangefs
        /mnt/nvme/llogan/write/generic
        ${SCRIPT_ROOT}/hostfiles/hostfile_servers_small
        ${SCRIPT_ROOT}/hostfiles/hostfile_clients)

#OrangeFS I/O-intensive test cases (ares)
function(prealloc nprocs storage_type caddr cport file_size)
    add_test(NAME prealloc_${nprocs}_${storage_type}_${file_size} COMMAND
            mpirun -n ${nprocs} ${CMAKE_CURRENT_BINARY_DIR}/benchmark
            -s ${storage_type} -caddr ${caddr} -cport ${cport} -path /test.bin
            -w prealloc -fs ${file_size})
endfunction()
function(io_test nprocs storage_type caddr cport block_size file_size total_io access_pattern wfrac rfrac config)
    add_test(NAME aresbm_io_${nprocs}_${storage_type}_${cport}_${block_size}_${file_size}_${total_io}_${access_pattern}_${wfrac}_${rfrac}_${config} COMMAND
            mpirun -n ${nprocs} ${CMAKE_CURRENT_BINARY_DIR}/benchmark
            -s ${storage_type} -caddr ${caddr} -cport ${cport} -path /test.bin
            -w io-only -ap ${access_pattern} -bs ${block_size} -fs ${file_size} -tot ${total_io} -wfrac ${wfrac} -rfrac ${rfrac} -seed 2542
            -out /home/llogan/io_test_${config}.csv)
endfunction()

set(PROCS 1 2 4 8 16 32 40)
set(STORAGE_TYPES orangefs mongodb redis)
set(ADDRS /mnt/nvme/llogan/write/generic 0 0)
set(PORTS 0 0 0)
set(FILE_SIZE 4g)
set(BLOCK_SIZES 1b 64b 512b 1k 4k 64k 256k 512k 1m 2m 4m 8m 16m)
set(TOTAL_IOS 256b 16k 128k 256k 1m 16m 64m 128m 256m 512m 1g 2g 4g)
set(ACCESS_PATTERNS seq uniform)
set(WFRACS 1 .5 0)
set(RFRACS 0 .5 1)
set(CONFIGS hdd4_basic hdd4_read hdd4_write ssd4_basic ssd4_read ssd4_write nvme4_basic hdd24_basic)

foreach(NPROC ${PROCS})
    list(LENGTH STORAGE_TYPES STORAGE_TYPES_LEN)
    math(EXPR STORAGE_TYPES_LAST "${STORAGE_TYPES_LEN} - 1")
    foreach(ST_IDX RANGE ${STORAGE_TYPES_LAST})
        list(GET STORAGE_TYPES ${ST_IDX} STORAGE_TYPE)
        list(GET ADDRS ${ST_IDX} ADDR)
        list(GET PORTS ${ST_IDX} PORT)
        prealloc(${NPROC} ${STORAGE_TYPE} ${ADDR} ${PORT} ${FILE_SIZE})
    endforeach()
endforeach()

foreach(NPROC ${PROCS})
    list(LENGTH STORAGE_TYPES STORAGE_TYPES_LEN)
    math(EXPR STORAGE_TYPES_LAST "${STORAGE_TYPES_LEN} - 1")
    foreach(ST_IDX RANGE ${STORAGE_TYPES_LAST})
        list(GET STORAGE_TYPES ${ST_IDX} STORAGE_TYPE)
        list(GET ADDRS ${ST_IDX} ADDR)
        list(GET PORTS ${ST_IDX} PORT)
        list(LENGTH BLOCK_SIZES BLOCK_SIZES_LEN)
        math(EXPR BLOCK_SIZES_LAST "${BLOCK_SIZES_LEN} - 1")
        foreach(BS_IDX RANGE ${BLOCK_SIZES_LAST})
            list(GET BLOCK_SIZES ${BS_IDX} BLOCK_SIZE)
            list(GET TOTAL_IOS ${BS_IDX} TOTAL_IO)
            foreach(ACCESS_PATTERN ${ACCESS_PATTERNS})
                list(LENGTH WFRACS WFRACS_LEN)
                math(EXPR WFRACS_LAST "${WFRACS_LEN} - 1")
                foreach(WF_IDX RANGE ${WFRACS_LAST})
                    list(GET WFRACS ${WF_IDX} WFRAC)
                    list(GET RFRACS ${WF_IDX} RFRAC)
                    foreach(CONFIG ${CONFIGS})
                        io_test(${NPROC} ${STORAGE_TYPE} ${ADDR} ${PORT} ${BLOCK_SIZE} ${FILE_SIZE} ${TOTAL_IO} ${ACCESS_PATTERN} ${WFRAC} ${RFRAC} ${CONFIG})
                    endforeach()
                endforeach()
            endforeach()
        endforeach()
    endforeach()
endforeach()

#OrangeFS metadata-intensive test cases (ares)
function(md_test nprocs storage_type caddr cport md_fcnt md_depth config)
    add_test(NAME aresbm_md_${nprocs}_${storage_type}_${cport}_${md_depth}_${md_fcnt}_${config} COMMAND
            mpirun -n ${nprocs} ${CMAKE_CURRENT_BINARY_DIR}/benchmark
            -s ${storage_type} -caddr ${caddr} -cport ${cport} -path /
            -w md-fs -md_fcnt ${md_fcnt} -md_depth ${md_depth}
            -out /home/llogan/md_test_${config}.csv)
endfunction()

set(PROCS 1 2 4 8 16 32 40)
set(STORAGE_TYPES orangefs mongodb redis)
set(ADDRS /mnt/nvme/llogan/write/generic 0 0)
set(PORTS 0 0 0)
set(FILE_SIZE 128g)
set(MD_FCNT 16k)
set(MD_DEPTHS 1 4 8)

foreach(NPROC ${PROCS})
    list(LENGTH STORAGE_TYPES STORAGE_TYPES_LEN)
    math(EXPR STORAGE_TYPES_LAST "${STORAGE_TYPES_LEN} - 1")
    foreach(ST_IDX RANGE ${STORAGE_TYPES_LAST})
        list(GET STORAGE_TYPES ${ST_IDX} STORAGE_TYPE)
        list(GET ADDRS ${ST_IDX} ADDR)
        list(GET PORTS ${ST_IDX} PORT)
        list(LENGTH BLOCK_SIZES BLOCK_SIZES_LEN)
        math(EXPR BLOCK_SIZES_LAST "${BLOCK_SIZES_LEN} - 1")
        foreach(MD_DEPTH ${MD_DEPTHS})
            foreach(CONFIG ${CONFIGS})
                md_test(${NPROC} ${STORAGE_TYPE} ${ADDR} ${PORT} ${MD_FCNT} ${MD_DEPTH} ${CONFIG})
            endforeach()
        endforeach()
    endforeach()
endforeach()

#Deploy nodes, and execute all tests for a particular configuration
set(DEV_TYPES hdd hdd hdd ssd ssd ssd nvme hdd)
set(CONFIGS hdd4_basic hdd4_read hdd4_write ssd4_basic ssd4_read ssd4_write nvme4_basic hdd24_basic)
set(CONF_FILES
        ${SCRIPT_ROOT}/conf/small/basic.conf ${SCRIPT_ROOT}/conf/small/read_optimize.conf ${SCRIPT_ROOT}/conf/small/write_optimize.conf
        ${SCRIPT_ROOT}/conf/ssd/basic.conf ${SCRIPT_ROOT}/conf/ssd/read_optimize.conf ${SCRIPT_ROOT}/conf/ssd/write_optimize.conf
        ${SCRIPT_ROOT}/conf/nvme/basic.conf ${SCRIPT_ROOT}/conf/big/basic.conf)
set(SERVER_HOST_FILES
        ${SCRIPT_ROOT}/hostfiles/hostfile_servers_small ${SCRIPT_ROOT}/hostfiles/hostfile_servers_small ${SCRIPT_ROOT}/hostfiles/hostfile_servers_small
        ${SCRIPT_ROOT}/hostfiles/hostfile_servers_ssd ${SCRIPT_ROOT}/hostfiles/hostfile_servers_ssd ${SCRIPT_ROOT}/hostfiles/hostfile_servers_ssd
        ${SCRIPT_ROOT}/hostfiles/hostfile_servers_nvme ${SCRIPT_ROOT}/hostfiles/hostfile_servers_big)
set(CLIENT_HOST_FILES
        ${SCRIPT_ROOT}/hostfiles/hostfile_clients ${SCRIPT_ROOT}/hostfiles/hostfile_clients2)
set(CLIENT_HOST_FILE_IDS
        clients1 clients2)

list(LENGTH CONFIGS CONFIGS_LEN)
math(EXPR CONFIGS_LAST "${CONFIGS_LEN} - 1")
foreach(CONFIG_IDX RANGE ${CONFIGS_LAST})
    list(GET DEV_TYPES ${CONFIG_IDX} DEV_TYPE)
    list(GET CONFIGS ${CONFIG_IDX} CONFIG)
    list(GET CONF_FILES ${CONFIG_IDX} CONF_FILE)
    list(GET SERVER_HOST_FILES ${CONFIG_IDX} SERVER_HOST_FILE)
    list(LENGTH CLIENT_HOST_FILES CLIENT_HOST_FILES_LEN)
    math(EXPR CLIENT_HOST_FILES_LAST "${CLIENT_HOST_FILES_LEN} - 1")
    foreach(CLIENT_HOST_IDX RANGE ${CLIENT_HOST_FILES_LAST})
        list(GET CLIENT_HOST_FILES ${CLIENT_HOST_IDX} CLIENT_HOST_FILE)
        list(GET CLIENT_HOST_FILE_IDS ${CLIENT_HOST_IDX} CLIENT_HOST_FILE_ID)
        add_custom_target(run_bms_${CONFIG}_${CLIENT_HOST_FILE_ID} COMMAND
            bash ${SCRIPT_ROOT}/experiments.sh ${DEV_TYPE} ${CONFIG} ${CONF_FILE} ${SERVER_HOST_FILE} ${CLIENT_HOST_FILE} ${SCRIPT_ROOT} ${CMAKE_BINARY_DIR})
    endforeach()
endforeach()
