#include "simdisk/disk-manager.h"

std::shared_ptr<DiskManager> DiskManager::instance;

DiskManager::DiskManager() {
    m_disk_path = std::string(get_current_dir_name()).append("/vdisk");
    m_initializor = std::make_unique<Initializor>(m_disk_path, DiskBlock::byte_size, block_count);
    m_fat = std::make_shared<FAT>();
    m_super_block = std::make_shared<SuperBlock>();

    for (int i = 0; i < 400; i++) {
        DiskBlock::bitset_to_chptr(readBlock(i).getData(), std::reinterpret_pointer_cast<char>(m_fat), sizeof(FAT), i * 1024);
    }

    DiskBlock::bitset_to_chptr(readBlock(super_block_id).getData(),
                               std::reinterpret_pointer_cast<char>(m_super_block),
                               sizeof(SuperBlock));
}

std::shared_ptr<DiskManager> DiskManager::getInstance() {
    if (instance == nullptr) {
        instance = std::shared_ptr<DiskManager>(new DiskManager());
    }
    return instance;
}

void DiskManager::initDisk() {
    m_initializor->init();
}

DiskBlock DiskManager::readBlock(int id) {
    std::ifstream ifs(m_disk_path, std::ios::binary | std::ios::in);

    if (!ifs.is_open())
        std::cerr << "could not open disk file" << std::endl;

    std::shared_ptr<char> buffer(new char[DiskBlock::byte_size]);
    ifs.seekg(id * DiskBlock::byte_size);
    ifs.read(buffer.get(), DiskBlock::byte_size);
    ifs.close();

    return DiskBlock(id, buffer);
}

void DiskManager::writeBlock(int id, DiskBlock block) {
    std::fstream fs(m_disk_path, std::ios::binary | std::ios::in | std::ios::out);

    if (!fs.is_open())
        std::cerr << "could not open disk file" << std::endl;

    std::shared_ptr<char> buffer(new char[DiskBlock::byte_size]);
    DiskBlock::bitset_to_chptr(block.getData(), buffer, DiskBlock::byte_size);
    fs.seekg(id * DiskBlock::byte_size);
    fs.write(buffer.get(), DiskBlock::byte_size);
    fs.close();
}