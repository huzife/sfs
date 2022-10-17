#include "simdisk/disk-manager.h"


std::unique_ptr<DiskManager> DiskManager::instance;

DiskManager::DiskManager() {
    m_disk_path = std::string(get_current_dir_name()).append("/vdisk");
    m_initializor = std::make_unique<Initializor>(m_disk_path);
}

std::unique_ptr<DiskManager>& DiskManager::getInstance() {
    if (instance == nullptr) {
        instance = std::make_unique<DiskManager>();
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
        
    char buffer[DiskBlock::byte_size];
    ifs.seekg(id * DiskBlock::byte_size);
    ifs.read(buffer, sizeof(buffer));
    ifs.close();

    return DiskBlock(id, buffer);
}

void DiskManager::writeBlock(int id, DiskBlock block) {
    std::fstream fs(m_disk_path, std::ios::binary | std::ios::in | std::ios::out);

    if (!fs.is_open())
        std::cerr << "could not open disk file" << std::endl;

    char buffer[DiskBlock::byte_size];
    DiskBlock::bitset_to_ptr(block.getData(), buffer);
    fs.seekg(id * DiskBlock::byte_size);
    fs.write(buffer, DiskBlock::byte_size);
    fs.close();
}