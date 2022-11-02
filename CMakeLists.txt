cmake_minimum_required(VERSION 3.16)
project(sfs VERSION 0.1)
set(CMAKE_CXX_STANDARD 20)

include_directories(include)

if(CMAKE_COMPILER_IS_GNUCXX)
    add_compile_options(-std=c++20)
    # add_compile_options(-O2)
    message(STATUS "optional: -std=c++20")
endif(CMAKE_COMPILER_IS_GNUCXX)

set(COMMAND_SOURCE
    src/simdisk/command/cd.cpp
    src/simdisk/command/del.cpp
    src/simdisk/command/dir.cpp
    src/simdisk/command/info.cpp
    src/simdisk/command/md.cpp
    src/simdisk/command/newfile.cpp
)

set(SIMDISK_SOURCE
    src/simdisk/main.cpp
    src/simdisk/initializor.cpp
    src/simdisk/disk-manager.cpp
    src/simdisk/basic.cpp
    ${COMMAND_SOURCE}
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)
add_executable(simdisk ${SIMDISK_SOURCE})