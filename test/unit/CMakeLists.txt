cmake_minimum_required(VERSION 3.0)
project (Symbios)

add_executable(github_test github_test.cpp)
add_test(NAME Github_test COMMAND github_test) 
