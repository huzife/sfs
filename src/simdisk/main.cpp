#include <iostream>
#include "simdisk/initializor.h"
#include "simdisk/disk-manager.h"

int main(int argc, char *argv[]) {
    system("clear");
    auto disk_manager = DiskManager::getInstance();
    disk_manager->initDisk();

    std::shared_ptr<FreeGroup> g = std::make_shared<FreeGroup>();
    DiskBlock::bitset_to_chptr(disk_manager->readBlock(std::strtol(argv[1], nullptr, 10)).getData(),
                               std::reinterpret_pointer_cast<char>(g), sizeof(FreeGroup));

    std::cout << g->m_count << std::endl;
    for (auto i : g->m_free_block) {
        std::cout << i << ' ';
    }
    std::cout << std::endl;

    return 0;
}