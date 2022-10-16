#include "simdisk/disk-manager.h"

std::unique_ptr<DiskManager> DiskManager::instance;

DiskManager::DiskManager() {
    m_disk_path = std::string(get_current_dir_name()).append("/vdisk");
}

std::unique_ptr<DiskManager>& DiskManager::getInstance() {
    if (instance == nullptr) {
        instance = std::make_unique<DiskManager>();
    }
    return instance;
}

void DiskManager::initDisk() {
    Initializor initializor(m_disk_path);
    initializor.init();
}

DiskBlock DiskManager::readBlock(int id) {
    std::ifstream ifs(m_disk_path, std::ios::binary | std::ios::in);

    if (!ifs.is_open())
        std::cerr << "could not open disk file" << std::endl;
    
    ifs.seekg(id * DiskBlock::byte_size);
    char buffer[DiskBlock::byte_size];
    ifs.read(buffer, sizeof(buffer));
    ifs.close();

    return DiskBlock(id, buffer);
}