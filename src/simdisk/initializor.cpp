#include "simdisk/initializor.h"
#include "simdisk/disk-manager.h"

void Initializor::init() {
    if (this->exist())
        std::cout << "file system already exists" << std::endl;
    else {
        std::cout << "file system does not exist, creating..." << std::endl;
        create();
    }
}

bool Initializor::exist() {
    std::ifstream ifs(m_path);
    return ifs.is_open();
}

void Initializor::create() {
    std::ofstream ofs(m_path, std::ios::out | std::ios::binary);

    // create an empty file and write 100 MiB
    // warn: must use loop instead of a 100 MiB char array!!!
    if (!ofs.is_open())
        std::cerr << "could not open disk file" << std::endl;

    char buffer[m_block_size];
    for (int i = 0; i < m_block_count; i++) {
        ofs.write(buffer, sizeof(buffer));
    }
    ofs.close();
    format();
}

void Initializor::format() {
    // TODO: initialize the FAT

    // initialize the free block
    for (int i = 400; i < m_block_count; i += FreeGroup::max_size) {
        auto g = std::make_shared<FreeGroup>();
        g->m_count = FreeGroup::max_size;
        for (int j = i + 1; j <= i + FreeGroup::max_size; j++) {
            g->m_free_block[j - i - 1] = j;
        }
        if (i + FreeGroup::max_size == DiskManager::block_count) {
            g->m_count = FreeGroup::max_size - 1;
            g->m_free_block[FreeGroup::max_size - 1] = -1;
        }
        DiskBlock block(i);
        block.setData(g);
        DiskManager::getInstance()->writeBlock(i, block);
    }
}