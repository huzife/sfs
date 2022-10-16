#include <iostream>
#include "simdisk/initializor.h"
#include "simdisk/disk-manager.h"

int main() {
    system("clear");
    auto& disk_manager = DiskManager::getInstance();
    disk_manager->initDisk();

    std::cerr << "[debug]: " << disk_manager->readBlock(0).m_data.to_string().size() << std::endl;

    return 0;
}