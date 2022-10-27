#include <iostream>
#include "simdisk/initializor.h"
#include "simdisk/disk-manager.h"

void DiskManager::test() {
    auto n = getIndexNode(m_super_block.m_root.m_inode);
    auto file = std::dynamic_pointer_cast<DirFile>(getFile(n));
    std::cout << file->m_size << std::endl;
    std::cout << file->m_subs << std::endl;
    std::cout << file->m_dirs.size() << std::endl;
    
    std::cout << file->m_parent.m_inode << std::endl;
    std::cout << file->m_parent.m_rec_len << std::endl;
    std::cout << (int)file->m_parent.m_name_len << std::endl;
    std::cout << file->m_parent.m_filename << std::endl;

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