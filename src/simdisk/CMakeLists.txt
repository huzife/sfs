include_directories(${CMAKE_SOURCE_DIR}/include/simdisk)

set(DISK_MANAGER_SOURCE disk-manager/disk-manager.cpp)

set(INITIALIZOR_SOURCE initializor/initializor.cpp)

add_executable(simdisk 
    ${DISK_MANAGER_SOURCE}
    ${INITIALIZOR_SOURCE}
    main.cpp
)