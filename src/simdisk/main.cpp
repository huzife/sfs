#include <iostream>
#include "simdisk/initializor.h"
#include "simdisk/disk-manager.h"

int main() {
    system("clear");
    auto& disk_manager = DiskManager::getInstance();
    disk_manager->initDisk();

    // auto block = DiskBlock(0);
    // for (int i = 0; i < DiskBlock::bit_size; i++) {
    //     block.m_data.set(i, i % 2);
    // }

    // disk_manager->writeBlock(0, block);

    return 0;
}