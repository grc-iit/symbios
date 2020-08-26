cmake_minimum_required(VERSION 3.10)
project(symbios)
set(INSTALL_DIR $ENV{INSTALL_DIR})
message(info ${INSTALL_DIR})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DH5_HAVE_DIRECT -w")
set(INCLUDE_DIRS "-I${INSTALL_DIR}/include/ -L${INSTALL_DIR}/lib/")# -DRPCLIB_ENABLE_LOGGING -DRPCLIB_ASIO=clmdep_asio -DRPCLIB_FMT=clmdep_fmt")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${INCLUDE_DIRS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${INCLUDE_DIRS}")

# External libraries symbios is dependent on.
option(COMPILE_HCL "Compile and install HCL" ON)
if(COMPILE_HCL)
    add_subdirectory(hcl)
endif()

# Debug flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHERMES_DEBUG")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHERMES_TIMER")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHERMES_TRACE")

# Source files
set(SYMBIOS_COMMON_SRC )
set(SYMBIOS_LIB_SRC ${SYMBIOS_COMMON_SRC} include/symbios/client.h src/client/client.cpp)

# add symbios lib
add_library(symbios SHARED ${SYMBIOS_LIB_SRC})
# TODO: add any required libraries

# add symbios server
set(SYMBIOS_SERVER_SRC ${SYMBIOS_COMMON_SRC} src/server/server.h src/server/main.cpp)
add_executable(symbios_server ${SYMBIOS_SERVER_SRC})
add_dependencies(symbios_server symbios)
# TODO: add any required libraries

option(BUILD_TEST "Build the unit tests" ON)
if(BUILD_TEST)
    enable_testing()
    include(CTest)
    add_subdirectory(test/unit)
    add_subdirectory(test/integration)
endif()







