#ifndef __DISK_MANAGER_H
#define __DISK_MANAGER_H

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <memory>
#include <string>
#include <assert.h>
#include <functional>
#include "simdisk/initializor.h"
#include "simdisk/basic.h"

// there can only be one disk manager, so I use singleton mode
class DiskManager {
public:
    static constexpr int disk_size = 100 * 1024 * 1024; // 100 MiB space
    static constexpr int block_count = disk_size / DiskBlock::byte_size; // 102400 blocks

private:
    static std::shared_ptr<DiskManager> instance;

    std::string m_disk_path;
    std::unique_ptr<Initializor> m_initializor;
    std::bitset<disk_size / DiskBlock::byte_size> m_blocks;

public:
    static std::shared_ptr<DiskManager> getInstance();

    void initDisk();

    DiskBlock readBlock(int id);

    void writeBlock(int id, DiskBlock block);

private:
    DiskManager();

};

#endif // __DISK_MANAGER_H