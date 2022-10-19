#include <iostream>
#include "simdisk/initializor.h"
#include "simdisk/disk-manager.h"

int main(int argc, char *argv[]) {
    system("clear");
    auto disk_manager = DiskManager::getInstance();
    disk_manager->initDisk();
    disk_manager->boot();


    // // test SuperBlock
    // auto s = std::make_shared<SuperBlock>();
    // std::shared_ptr<char[]> buffer(new char[DiskManager::block_size]);
    // disk_manager->readBlock(400).getData(buffer);
    // s->load(buffer, DiskManager::block_size);
    // std::cout << s->m_count << std::endl;
    // for (auto i : s->m_free_block) {
    //     std::cout << i << ' ';
    // }
    // std::cout << std::endl;

    // test FAT
    auto f = std::make_shared<FAT>();
    std::shared_ptr<char[]> buffer(new char[DiskManager::fat_size]);
    for (int i = 0; i < 400; i++) {
        disk_manager->readBlock(i).getData(buffer, i * DiskManager::block_size);
    }
    f->load(buffer, DiskManager::fat_size);
    for (auto i : f->m_table) {
        std::cout << i << ' ';
    }
    std::cout << std::endl;

    return 0;
}