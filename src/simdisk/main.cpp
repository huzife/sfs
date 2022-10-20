#include <iostream>
#include "simdisk/initializor.h"
#include "simdisk/disk-manager.h"

int main(int argc, char *argv[]) {
    system("clear");
    auto disk_manager = DiskManager::getInstance();
    disk_manager->initDisk();
    disk_manager->boot();

    std::shared_ptr<char[]> buffer(new char[DiskManager::block_size * 12]);
    for (int i = 0; i < 12; i++) {
        disk_manager->readBlock(i + 401).getData(buffer, i * DiskManager::block_size);
    }
    auto m = std::make_shared<AllocMap<DiskManager::file_block_count>>();
    m->load(buffer, DiskManager::block_size * 12);
    std::cout << m->m_map.size() << std::endl;
    std::cout << sizeof(m->m_map) << std::endl;
    std::cout << m->m_map.to_string() << std::endl;

    return 0;
}