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
add_custom_target(bm-prealloc COMMAND
        ${CMAKE_CURRENT_BINARY_DIR}/benchmark
        -s orangefs -caddr ~/ -path test3.bin
        -w prealloc -fs 1b)
add_custom_target(bm-rw-test-seq COMMAND
        ${CMAKE_CURRENT_BINARY_DIR}/benchmark
        -s orangefs -caddr ~/ -path test.bin
        -w io-only -bs 4k -fs 0k -tot 1g -wfrac .5 -rfrac .5 -ap uniform -seed 2542
        -out ~/test.csv)
add_custom_target(bm-rw-test-rand COMMAND
        ${CMAKE_CURRENT_BINARY_DIR}/benchmark
        -s orangefs -caddr ~/ -path test.bin
        -w io-only -bs 4k -fs 0k -tot 1g -wfrac .5 -rfrac .5 -ap uniform -seed 2542
        -out ~/test.csv)
add_custom_target(bm-md-test COMMAND
        ${CMAKE_CURRENT_BINARY_DIR}/benchmark
        -s orangefs -caddr ~/ -path test.bin
        -w md-test -md_depth 20 -md_fcnt 20
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
    add_test(NAME aresbm_${nprocs}_${storage_type}_${cport}_${block_size}_${file_size}_${total_io}_${access_pattern}_${wfrac}_${rfrac}_${config} COMMAND
            mpirun -n ${nprocs} ${CMAKE_CURRENT_BINARY_DIR}/benchmark
            -s ${storage_type} -caddr ${caddr} -cport ${cport} -path /test.bin
            -w io-only -ap ${access_pattern} -bs ${block_size} -fs ${file_size} -tot ${total_io} -wfrac ${wfrac} -rfrac ${rfrac} -seed 2542
            -out ~/test_${config}.csv)
endfunction()

set(PROCS 1 2 4 8 16 32 40)
set(STORAGE_TYPES orangefs mongodb redis)
set(ADDRS /mnt/nvme/llogan/write/generic 0 0)
set(PORTS 0 0 0)
set(BLOCK_SIZES 1b 64b 512b 1k 4k 64k 256k 512k 1m 2m 4m 8m 16m)
set(FILE_SIZES 128g 128g 128g 128g 128g 128g 128g 128g 128g 128g 128g 128g 128g)
set(TOTAL_IOS 256k 512k 1m 2m 128m 2g 8g 16g 32g 128g 128g 128g 128g)
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
        list(LENGTH BLOCK_SIZES BLOCK_SIZES_LEN)
        math(EXPR BLOCK_SIZES_LAST "${BLOCK_SIZES_LEN} - 1")
        foreach(BS_IDX RANGE ${BLOCK_SIZES_LAST})
            list(GET BLOCK_SIZES ${BS_IDX} BLOCK_SIZE)
            list(GET FILE_SIZES ${BS_IDX} FILE_SIZE)
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

foreach(NPROC ${PROCS})
    list(LENGTH STORAGE_TYPES STORAGE_TYPES_LEN)
    math(EXPR STORAGE_TYPES_LAST "${STORAGE_TYPES_LEN} - 1")
    foreach(ST_IDX RANGE ${STORAGE_TYPES_LAST})
        list(GET STORAGE_TYPES ${ST_IDX} STORAGE_TYPE)
        list(GET ADDRS ${ST_IDX} ADDR)
        list(GET PORTS ${ST_IDX} PORT)
        prealloc(${NPROC} ${STORAGE_TYPE} ${ADDR} ${PORT} 128g)
    endforeach()
endforeach()

#OrangeFS metadata-intensive test cases (ares)

