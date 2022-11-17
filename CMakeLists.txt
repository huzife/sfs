cmake_minimum_required(VERSION 3.16)
project(sfs VERSION 0.1)
set(CMAKE_CXX_STANDARD 20)

include_directories(include)

if(CMAKE_COMPILER_IS_GNUCXX)
    add_compile_options(-std=c++20)
    # add_compile_options(-lpthread)
    # add_compile_options(-O2)
    message(STATUS "optional: -std=c++20")
    # message(STATUS "optional: -lpthread")
endif(CMAKE_COMPILER_IS_GNUCXX)

set(COMMAND_SOURCE
    src/simdisk/command/cd.cpp
    src/simdisk/command/copy.cpp
    src/simdisk/command/del.cpp
    src/simdisk/command/dir.cpp
    src/simdisk/command/info.cpp
    src/simdisk/command/md.cpp
    src/simdisk/command/newfile.cpp
    src/simdisk/command/rd.cpp
)

set(SIMDISK_SOURCE
    src/simdisk/main.cpp
    src/simdisk/initializor.cpp
    src/simdisk/disk-manager.cpp
    src/simdisk/basic.cpp
    ${COMMAND_SOURCE}
)

set(SIMSH_SOURCE
    src/simsh/main.cpp
    src/simsh/simsh.cpp
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)
add_executable(simdisk ${SIMDISK_SOURCE})
add_executable(simsh ${SIMSH_SOURCE})

target_link_libraries(simdisk pthread ncurses)
