#include <iostream>
#include "simdisk/initializor.h"
#include "simdisk/disk-manager.h"

int main(int argc, char *argv[]) {
    system("clear");
    auto disk_manager = DiskManager::getInstance();
    disk_manager->initDisk();

    // int id = 0;
    // if (argc == 2) id = std::strtol(argv[1], nullptr, 10);

    // // test superblock
    // std::shared_ptr<SuperBlock> g = std::make_shared<SuperBlock>();
    // DiskBlock::bitset_to_chptr(disk_manager->readBlock(id).getData(),
    //                            std::reinterpret_pointer_cast<char>(g), sizeof(SuperBlock));

    // std::cout << g->m_count << std::endl;
    // for (auto i : g->m_free_block) {
    //     std::cout << i << ' ';
    // }
    // std::cout << std::endl;

    // test FAT
    std::shared_ptr<FAT> f = std::make_shared<FAT>();
    for (int i = 0; i < 400; i++) {
        DiskBlock::bitset_to_chptr(disk_manager->readBlock(i).getData(), std::reinterpret_pointer_cast<char>(f), sizeof(FAT), i * 1024);
    }
    for (int i = 0; i < 102400; i++) {
        std::cout << f->m_table[i] << ' ';
    }
    std::cout << std::endl;

    return 0;
}