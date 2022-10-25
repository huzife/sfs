#include <iostream>
#include "simdisk/initializor.h"
#include "simdisk/disk-manager.h"

void DiskManager::test() {
    // std::cout << m_super_block.m_block_map_location << std::endl;
    // std::cout << m_super_block.m_block_map_size << std::endl;
    // std::cout << m_super_block.m_block_size << std::endl;
    // std::cout << m_super_block.m_dirt << std::endl;
    // std::cout << m_super_block.m_fat_location << std::endl;
    // std::cout << m_super_block.m_fat_size << std::endl;
    // std::cout << m_super_block.m_filename_maxbytes << std::endl;
    
    // for (auto i : m_fat.m_table) {
    //     std::cout << i << ' ';
    // }
    std::cout << m_inode_map.m_map << std::endl;
    std::cout << m_block_map.m_map << std::endl;
    std::cout << std::endl;
}

int main(int argc, char *argv[]) {
    system("clear");
    auto disk_manager = DiskManager::getInstance();
    disk_manager->initDisk();
    disk_manager->boot();

    disk_manager->test();

    return 0;
}