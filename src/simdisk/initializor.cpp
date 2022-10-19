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
    // initialize the FAT
    auto t = std::make_shared<FAT>();
    t->m_table.resize(DiskManager::block_count, -1);
    for (int i = 0; i < DiskManager::super_block_id - 1; i++) {
        t->m_table[i] = i + 1;
    }

    // get the pointer
    auto chptr = t->dump();
    for (int i = 0; i < DiskManager::super_block_id; i++) {
        DiskBlock block(i);
        block.setData(chptr, i * DiskManager::block_size);
        DiskManager::getInstance()->writeBlock(i, block);
    }

    // initialize the super block
    for (int i = DiskManager::super_block_id; i < m_block_count; i += SuperBlock::max_size) {
        auto g = std::make_shared<SuperBlock>();
        g->m_count = SuperBlock::max_size;
        for (int j = SuperBlock::max_size; j > 0; j--) {
            g->m_free_block.emplace_back(i + j);
        }

        if (i + SuperBlock::max_size == DiskManager::block_count) {
            g->m_count = SuperBlock::max_size - 1;
            g->m_free_block.back() = -1;
        }
        DiskBlock block(i);
        block.setData(g->dump());
        DiskManager::getInstance()->writeBlock(i, block);
    }
}