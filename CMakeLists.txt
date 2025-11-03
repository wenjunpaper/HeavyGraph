cmake_minimum_required(VERSION 3.16)
project(HeavyGraph)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O3")
set(CMAKE_BUILD_TYPE Debug)
include_directories(.)
include_directories(Common)
include_directories(Dataset)
include_directories(Detector)
include_directories(GSS)
include_directories(Ours)

add_executable(main main.cpp)