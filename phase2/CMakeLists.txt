cmake_minimum_required(VERSION 3.20)
project(YellowTaxiPhase2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)   # -O3, so timings are fair
endif()

find_package(OpenMP REQUIRED)

# shared code: splitting the file between threads, reading and parsing the lines
add_library(common STATIC common/csv_reader.cpp)
target_include_directories(common PUBLIC common)
target_link_libraries(common PUBLIC OpenMP::OpenMP_CXX)

# step 1: phase 1 objects + threads
add_executable(taxi_threads step1_threads/main.cpp step1_threads/trip_data.cpp)
target_include_directories(taxi_threads PRIVATE step1_threads)
target_link_libraries(taxi_threads PRIVATE common)

# step 2: the compact data is its own library, and the app links to it
add_library(taxistore STATIC step2_compact/trip_store.cpp)
target_include_directories(taxistore PUBLIC step2_compact)
target_link_libraries(taxistore PUBLIC common)

add_executable(taxi_compact step2_compact/main.cpp)
target_link_libraries(taxi_compact PRIVATE taxistore)
