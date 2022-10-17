cmake_minimum_required(VERSION 3.16)
project(sfs VERSION 0.1)
set(CMAKE_CXX_STANDARD 20)

include_directories(include)

set(SIMDISK_SOURCE
    src/simdisk/main.cpp
    src/simdisk/initializor.cpp
    src/simdisk/disk-manager.cpp
    src/simdisk/basic.cpp
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)
add_executable(simdisk ${SIMDISK_SOURCE})