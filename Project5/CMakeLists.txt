cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 20)

project(cppMat)

include_directories(./include)

aux_source_directory(./src DIR_SRCS)

ADD_EXECUTABLE(cppMat ${DIR_SRCS})

#add_definitions(-DWITH_NEON)
add_definitions(-DWITH_AVX2 -fopenmp -lgomp)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND)
    message("OpenMP found.")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()