cmake_minimum_required(VERSION 3.0)

project(TinyServer CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -march=native -O3 -pthread" )

aux_source_directory(./ SOURCESLIST)

set(CMAKE_CXX_COMPILER "g++")

add_executable(${PROJECT_NAME} ${SOURCESLIST})

