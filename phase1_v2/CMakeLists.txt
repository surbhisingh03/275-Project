cmake_minimum_required(VERSION 3.20)
project(YellowTaxiPhase1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Always build optimised (-O3) unless told otherwise, so timings are fair.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(taxi_phase1 src/main.cpp src/csv_reader.cpp src/trip_data.cpp)
target_include_directories(taxi_phase1 PRIVATE include)
