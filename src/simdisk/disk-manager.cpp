#include "simdisk/disk-manager.h"

std::shared_ptr<DiskManager> DiskManager::instance;

DiskManager::DiskManager() {
    m_disk_path = std::string(get_current_dir_name()).append("/vdisk");
    m_initializor = std::make_unique<Initializor>(m_disk_path, DiskManager::block_size, block_count);
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

void DiskManager::boot() {
    std::shared_ptr<char[]> fat_data(new char[fat_size]);
    for (int i = 0; i < 400; i++) {
        readBlock(i);
        readBlock(i).getData(fat_data, i * DiskManager::block_size);
    }
    m_fat.load(fat_data, fat_size);

    std::shared_ptr<char[]> super_block_data(new char[DiskManager::block_size]);
    readBlock(super_block_id).getData(super_block_data);
    m_super_block.load(super_block_data, DiskManager::block_size);
}

DiskBlock DiskManager::readBlock(int id) {
    std::ifstream ifs(m_disk_path, std::ios::binary | std::ios::in);

    if (!ifs.is_open())
        std::cerr << "could not open disk file" << std::endl;

    DiskBlock ret(id);
    std::shared_ptr<char[]> buffer(new char[DiskManager::block_size]);
    ifs.seekg(id * DiskManager::block_size);
    ifs.read(buffer.get(), DiskManager::block_size);
    ifs.close();

    ret.setData(buffer);

    return ret;
}

void DiskManager::writeBlock(int id, DiskBlock block) {
    std::fstream fs(m_disk_path, std::ios::binary | std::ios::in | std::ios::out);

    if (!fs.is_open())
        std::cerr << "could not open disk file" << std::endl;

    std::shared_ptr<char[]> buffer(new char[DiskManager::block_size]);
    block.getData(buffer);
    fs.seekg(id * DiskManager::block_size);
    fs.write(buffer.get(), DiskManager::block_size);
    fs.close();
}