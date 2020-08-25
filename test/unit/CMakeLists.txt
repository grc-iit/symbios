cmake_minimum_required(VERSION 3.10)
set(INSTALL_DIR $ENV{INSTALL_DIR})
include_directories(${INSTALL_DIR}/include/)
set(CMAKE_CXX_STANDARD 17)


set(LIBS -L${INSTALL_DIR}/lib64 -L${INSTALL_DIR}/lib -L${INSTALL_DIR}/lib -lmpi -lpthread -ldl)
include_directories(${CMAKE_SOURCE_DIR})
set(CTEST_ENVIRONMENT "LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/")
set($ENV{LD_PRELOAD} "$ENV{LD_PRELOAD} ${CMAKE_BINARY_DIR}")

set(test_files  client
                data_placement
                dynamic_provisioner
                io_clients
                metadata_orchestrator
                server
                storage_cost_predictor)
foreach (test_file ${test_files})
    set(exec ${test_file})
    add_executable(${exec} ${test_file}/test.cpp)
    add_dependencies(${exec} symbios)
    target_include_directories(${exec} PRIVATE "${CMAKE_BINARY_DIR}/")
    target_link_libraries(${exec} ${LIBS})
    target_link_libraries(${exec} symbios)
    set_target_properties(${exec} PROPERTIES FOLDER test/integration)
endforeach ()