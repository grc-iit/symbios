cmake_minimum_required(VERSION 3.10)
project(symbios VERSION 1.0.0 DESCRIPTION "Symbios")
set(INSTALL_DIR $ENV{INSTALL_DIR})
#string(REPLACE ":" "/;" MODULE_LIST $ENV{PATH})
#string(REPLACE ":" "/;" MODULE_LIB_LIST $ENV{LD_LIBRARY_PATH})
#message(install_dir: ${INSTALL_DIR})
#message(module_lsit ${MODULE_LIST})
#message(module_lib_list ${MODULE_LIB_LIST})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DH5_HAVE_DIRECT -w -DBASKET_ENABLE_RPCLIB")
set(INCLUDE_DIRS "-I${INSTALL_DIR}/include/ -L${INSTALL_DIR}/lib/") #-L${PROJECT_SOURCE_DIR}/build/hcl/ -I${PROJECT_SOURCE_DIR}/hcl/include/") # -I${MODULE_LIST} -L${MODULE_LIB_LIST}")# -DRPCLIB_ENABLE_LOGGING -DRPCLIB_ASIO=clmdep_asio -DRPCLIB_FMT=clmdep_fmt")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${INCLUDE_DIRS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${INCLUDE_DIRS}")
configure_file(symbios.pc.in symbios.pc @ONLY)

if(CMAKE_BUILD_TYPE MATCHES Release)
    set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ffast-math")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ffast-math")
endif()

include_directories(include)
include_directories(common/include)
include_directories(src)

set(CMAKE_CXX_STANDARD 17)

# Debug flags
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCOMMON_DEBUG_MSG")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCOMMON_DEBUG_TIMER")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCOMMON_DEBUG_TRACE")

# Source files
set(SYMBIOS_COMMON_SRC ${CMAKE_SOURCE_DIR}/include/symbios/common/data_structure.h ${CMAKE_SOURCE_DIR}/include/symbios/common/configuration_manager.h ${CMAKE_SOURCE_DIR}/src/symbios/common/data_structure.cpp ${CMAKE_SOURCE_DIR}/common/src/common/debug.cpp
        ${CMAKE_SOURCE_DIR}/common/include/common/debug.h ${CMAKE_SOURCE_DIR}/common/include/common/arguments.h)
set(SYMBIOS_LIB_SRC ${SYMBIOS_COMMON_SRC} include/symbios/client/client.h src/symbios/client/client.cpp
        src/symbios/client/posix.cpp include/symbios/client/posix.h)

# add symbios lib
add_library(symbios SHARED ${SYMBIOS_LIB_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER include/symbios/symbios.h)
set(CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_PREFIX}/include)
set(CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX}/lib)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

target_link_libraries(symbios -lbasket -lrpc  -lboost_system -lboost_filesystem -ldl)
# TODO: add any required libraries

# add symbios server

set(SYMBIOS_SERVER_SRC ${SYMBIOS_COMMON_SRC}
        ${CMAKE_SOURCE_DIR}/src/symbios/storage_cost_predictor/storage_cost_predictor.cpp ${CMAKE_SOURCE_DIR}/src/symbios/storage_cost_predictor/storage_cost_predictor.h
        ${CMAKE_SOURCE_DIR}/src/symbios/server/server.h ${CMAKE_SOURCE_DIR}/src/symbios/server/server.cpp
        ${CMAKE_SOURCE_DIR}/src/symbios/io_clients/io.h ${CMAKE_SOURCE_DIR}/src/symbios/io_clients/io_factory.h
        ${CMAKE_SOURCE_DIR}/src/symbios/io_clients/file_io.h ${CMAKE_SOURCE_DIR}/src/symbios/io_clients/file_io.cpp
        ${CMAKE_SOURCE_DIR}/src/symbios/io_clients/redis_io.h ${CMAKE_SOURCE_DIR}/src/symbios/io_clients/redis_io.cpp
        ${CMAKE_SOURCE_DIR}/src/symbios/io_clients/mongo_io.h ${CMAKE_SOURCE_DIR}/src/symbios/io_clients/mongo_io.cpp
        ${CMAKE_SOURCE_DIR}/src/symbios/data_distribution/data_distribution.h ${CMAKE_SOURCE_DIR}/src/symbios/data_distribution/data_distribution_factory.h
        ${CMAKE_SOURCE_DIR}/src/symbios/data_distribution/dp_dde.h ${CMAKE_SOURCE_DIR}/src/symbios/data_distribution/dp_dde.cpp
        ${CMAKE_SOURCE_DIR}/src/symbios/data_distribution/heuristics_dde.h ${CMAKE_SOURCE_DIR}/src/symbios/data_distribution/heuristics_dde.cpp
        ${CMAKE_SOURCE_DIR}/src/symbios/data_distribution/random_dde.h ${CMAKE_SOURCE_DIR}/src/symbios/data_distribution/random_dde.cpp
        ${CMAKE_SOURCE_DIR}/src/symbios/data_distribution/round_robin_dde.h ${CMAKE_SOURCE_DIR}/src/symbios/data_distribution/round_robin_dde.cpp
        ${CMAKE_SOURCE_DIR}/src/symbios/metadata_orchestrator/metadata_orchestrator.h ${CMAKE_SOURCE_DIR}/src/symbios/metadata_orchestrator/metadata_orchestrator.cpp
        ${CMAKE_SOURCE_DIR}/src/symbios/server/daemon.h)

add_library(symbios_server_lib SHARED ${SYMBIOS_SERVER_SRC})
add_dependencies(symbios_server_lib symbios)
target_link_libraries(symbios_server_lib -lmpi -lhiredis -lbasket -lrpc -lredis++ -lmongoc-1.0 -lbson-1.0 -ldlib -lopenblas -lgfortran -lm -lquadmath  -lbsoncxx -lboost_system -lboost_filesystem  -lmongocxx -lpthread -ldl)

add_executable(symbios_server ${CMAKE_SOURCE_DIR}/src/symbios/server/main.cpp)
add_dependencies(symbios_server symbios)
add_dependencies(symbios_server symbios_server_lib)
target_link_libraries(symbios_server symbios_server_lib)
target_link_libraries(symbios_server  -lmpi)


add_custom_target(copy_conf)
add_custom_command(
        TARGET copy_conf
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/conf
        ${CMAKE_BINARY_DIR})

add_custom_target(copy_scripts)
add_custom_command(
        TARGET copy_scripts
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/scripts
        ${CMAKE_BINARY_DIR})

add_dependencies(symbios copy_conf)
add_dependencies(symbios copy_scripts)

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

if (DEFINED ENV{PKG_CONFIG_PATH})
   install(FILES ${CMAKE_BINARY_DIR}/symbios.pc
                  DESTINATION $ENV{PKG_CONFIG_PATH}/pkgconfig)
else()
   install(FILES ${CMAKE_BINARY_DIR}/symbios.pc
                  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)
endif()

# add benchmark tool
add_subdirectory(benchmark)
add_subdirectory(common/test)

option(BUILD_TEST "Build the unit tests" ON)
if(BUILD_TEST)
    enable_testing()
    include(CTest)
    add_subdirectory(test/unit)
    add_subdirectory(test/integration)
endif()







