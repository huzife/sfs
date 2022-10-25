#include <iostream>
#include "simdisk/initializor.h"
#include "simdisk/disk-manager.h"

void DiskManager::test() {
    auto n = getIndexNode(m_super_block.m_root.m_inode);
    std::cout << timeToDate(n->m_create_time) << std::endl;
    std::cout << timeToDate(n->m_access_time) << std::endl;
    std::cout << timeToDate(n->m_modify_time) << std::endl;
    std::cout << timeToDate(n->m_change_time) << std::endl;
}

int main(int argc, char *argv[]) {
    system("clear");
    auto disk_manager = DiskManager::getInstance();
    disk_manager->initDisk();
    disk_manager->boot();

    disk_manager->test();

    return 0;
}