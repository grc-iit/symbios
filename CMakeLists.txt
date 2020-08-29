cmake_minimum_required(VERSION 3.10)
project(symbios)
set(INSTALL_DIR $ENV{INSTALL_DIR})
#string(REPLACE ":" "/;" MODULE_LIST $ENV{PATH})
#string(REPLACE ":" "/;" MODULE_LIB_LIST $ENV{LD_LIBRARY_PATH})
#message(install_dir: ${INSTALL_DIR})
#message(module_lsit ${MODULE_LIST})
#message(module_lib_list ${MODULE_LIB_LIST})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DH5_HAVE_DIRECT -w")
set(INCLUDE_DIRS "-I${INSTALL_DIR}/include/ -L${INSTALL_DIR}/lib/") #-L${PROJECT_SOURCE_DIR}/build/hcl/ -I${PROJECT_SOURCE_DIR}/hcl/include/") # -I${MODULE_LIST} -L${MODULE_LIB_LIST}")# -DRPCLIB_ENABLE_LOGGING -DRPCLIB_ASIO=clmdep_asio -DRPCLIB_FMT=clmdep_fmt")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${INCLUDE_DIRS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${INCLUDE_DIRS}")

include_directories(include)
include_directories(src)

# External libraries hermes is dependent on.
option(COMPILE_HCL "Compile and install HCL" ON)
if(COMPILE_HCL)
    add_subdirectory(hcl)
    include_directories(hcl/include)
endif()

set(CMAKE_CXX_STANDARD 17)

# Debug flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHERMES_DEBUG")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHERMES_TIMER")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHERMES_TRACE")

# Source files
set(SYMBIOS_COMMON_SRC include/symbios/common/data_structure.h include/symbios/common/configuration_manager.h)
set(SYMBIOS_LIB_SRC ${SYMBIOS_COMMON_SRC} include/symbios/client/client.h src/symbios/client/client.cpp
        src/symbios/client/posix.cpp )

# add symbios lib
add_library(symbios SHARED ${SYMBIOS_LIB_SRC})
add_dependencies(symbios basket)
target_link_libraries(symbios basket)
target_link_libraries(symbios -lmpi -lhiredis -lredis++ -lmongoc-1.0 -lbson-1.0 -lbsoncxx -lboost_system -lboost_filesystem  -lmongocxx -lpthread)
# TODO: add any required libraries

# add symbios server
set(SYMBIOS_SERVER_SRC ${SYMBIOS_COMMON_SRC} src/symbios/server/main.cpp
        src/symbios/server/server.h src/symbios/server/server.cpp
        src/symbios/io_clients/io.h src/symbios/io_clients/io_factory.h
        src/symbios/io_clients/file_io.h src/symbios/io_clients/file_io.cpp
        src/symbios/io_clients/object_store_io.h src/symbios/io_clients/object_store_io.cpp
        src/symbios/io_clients/redis_io.h src/symbios/io_clients/redis_io.cpp
        src/symbios/io_clients/mongo_io.h src/symbios/io_clients/mongo_io.cpp
        src/symbios/server/daemon.cpp src/symbios/server/daemon.h)

add_executable(symbios_server ${SYMBIOS_SERVER_SRC})
add_dependencies(symbios_server symbios)
add_dependencies(symbios_server basket)
target_link_libraries(symbios_server basket)
# TODO: add any required libraries

# add benchmark tool
add_subdirectory(benchmark)

option(BUILD_TEST "Build the unit tests" ON)
if(BUILD_TEST)
    enable_testing()
    include(CTest)
    add_subdirectory(test/unit)
    add_subdirectory(test/integration)
endif()







