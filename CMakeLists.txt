cmake_minimum_required(VERSION 3.0)
project (Symbios)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
	include(CTest)
endif()
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
	add_subdirectory(test)
endif()
