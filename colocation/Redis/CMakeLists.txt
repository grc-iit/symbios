cmake_minimum_required(VERSION 3.13)
project(Redis C)

set(CMAKE_C_COMPILER /home/kfeng/MPICH/bin/mpicc)

include_directories(/home/kfeng/MPICH/include)
include_directories(/home/kfeng/install/include)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEBUG -g")

set(MPI true)

# Add stuff for Redis
include_directories(/home/kfeng/pkg_src/hiredis-vip)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -L/home/kfeng/pkg_src/hiredis-vip -lhiredis_vip")

# Require MPI for this project:
if(DEFINED MPI)
    #find_package(MPI REQUIRED)
    set(MPI_INCLUDE_PATH "/home/kfeng/MPICH/include")
    set(MPI_LINK_FLAGS "-L/home/kfeng/MPICH/lib -lmpi")
    set(CMAKE_C_COMPILE_FLAGS ${CMAKE_C_COMPILE_FLAGS} ${MPI_COMPILE_FLAGS})
    set(CMAKE_C_LINK_FLAGS ${CMAKE_C_LINK_FLAGS} ${MPI_LINK_FLAGS})
    include_directories(${MPI_INCLUDE_PATH})
endif()

set(SOURCE_FILES main.c redis.c redis.h)

add_executable(Redis ${SOURCE_FILES})