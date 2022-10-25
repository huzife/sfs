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
        disk_manager->readBlock(i + 413).getData(buffer, DiskManager::block_size, 0, i * DiskManager::block_size);
    }
    auto m = std::make_shared<AllocMap<DiskManager::file_block_count>>();
    m->load(buffer, DiskManager::block_size * 12);
    std::cout << m->m_map.size() << std::endl;
    std::cout << sizeof(m->m_map) << std::endl;
    std::cout << m->m_map.to_string() << std::endl;
    std::cout << m->operator[](0) << std::endl;

    // std::shared_ptr<char[]> buffer(new char[1024]);
    // disk_manager->readBlock(0).getData(buffer);
    // SuperBlock b;
    // b.load(buffer, 1024);
    // std::cout << b.m_block_map_location << std::endl;
    // std::cout << b.m_block_map_size << std::endl;
    // std::cout << b.m_fat_location << std::endl;
    // std::cout << b.m_fat_size << std::endl;
    // std::cout << b.m_inode_map_location << std::endl;
    // std::cout << b.m_inode_map_size << std::endl;
    // std::cout << b.m_free_block << std::endl;
    // std::cout << b.m_root.m_filename << std::endl;
    // std::cout << b.m_root.m_inode << std::endl;
    // std::cout << (int)b.m_root.m_name_len << std::endl;
    // std::cout << b.m_root.m_rec_len << std::endl;

    // std::shared_ptr<char[]> buffer(new char[1024]);
    // disk_manager->readBlock(DiskManager::inode_block_offset).getData(buffer);
    // IndexNode n;
    // n.load(buffer, 1024);
    // std::cout << n.m_owner << std::endl;
    // std::cout << n.m_count << std::endl;
    // std::cout << n.m_create_time.time_since_epoch().count() << std::endl;

    // std::shared_ptr<char[]> buffer(new char[1024]);
    // disk_manager->readBlock(DiskManager::file_block_offset).getData(buffer);
    // DirFile d;
    // d.load(buffer, 2);
    // std::cout << d.m_parent.m_filename << std::endl;
    // std::cout << d.m_current.m_filename << std::endl;
    // std::cout << d.m_dirs.size() << std::endl;

    return 0;
}