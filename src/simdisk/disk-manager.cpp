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

DiskBlock DiskManager::readBlock(int id) {
    std::ifstream ifs(m_disk_path, std::ios::binary | std::ios::in);

    if (!ifs.is_open())
        std::cerr << "could not open disk file" << std::endl;

    DiskBlock ret(id);
    std::shared_ptr<char[]> buffer(new char[DiskManager::block_size]);
    ifs.seekg(id * DiskManager::block_size);
    ifs.read(buffer.get(), DiskManager::block_size);
    ifs.close();

    ret.setData(buffer, 0);

    return ret;
}

void DiskManager::writeBlock(int id, DiskBlock block) {
    std::fstream fs(m_disk_path, std::ios::binary | std::ios::in | std::ios::out);

    if (!fs.is_open())
        std::cerr << "could not open disk file" << std::endl;

    std::shared_ptr<char[]> buffer(new char[DiskManager::block_size]);
    block.getData(buffer, 0);
    fs.seekg(id * DiskManager::block_size);
    fs.write(buffer.get(), DiskManager::block_size);
    fs.close();
}

void DiskManager::initDisk() {
    m_initializor->init();
}

void DiskManager::boot() {
    loadSuperBlock();
    loadFAT();
    loadINodeMap();
    loadBlockMap();
}

void DiskManager::loadSuperBlock() {
    std::shared_ptr<char[]> buffer(new char[DiskManager::block_size]);
    readBlock(super_block_id).getData(buffer, 0);
    m_super_block.load(buffer);
    m_super_block.m_dirt = false;
}

void DiskManager::loadFAT() {
    std::shared_ptr<char[]> buffer(new char[m_super_block.m_fat_size * DiskManager::block_size]);
    for (int i = 0; i < m_super_block.m_fat_size; i++) {
        readBlock(i + m_super_block.m_fat_location).getData(buffer, i * DiskManager::block_size);
    }
    
    m_fat.load(buffer);
}

void DiskManager::loadINodeMap() {
    std::shared_ptr<char[]> buffer(new char[m_super_block.m_inode_map_size * DiskManager::block_size]);
    for (int i = 0; i < m_super_block.m_inode_map_size; i++) {
        readBlock(i + m_super_block.m_inode_map_location).getData(buffer, i * DiskManager::block_size);
    }
    m_inode_map.load(buffer);
}

void DiskManager::loadBlockMap() {
    std::shared_ptr<char[]> buffer(new char[m_super_block.m_block_map_size * DiskManager::block_size]);
    for (int i = 0; i < m_super_block.m_block_map_size; i++) {
        readBlock(i + m_super_block.m_block_map_location).getData(buffer, i * DiskManager::block_size);
    }
    m_block_map.load(buffer);
}

IndexNode DiskManager::getIndexNode(int id) {
    assert(id < m_super_block.m_total_inode);
    IndexNode ret;
    std::shared_ptr<char[]> buffer(new char[DiskManager::inode_size]);
    int b_id = id / DiskManager::inode_per_block + DiskManager::inode_block_offset;
    int offs = id % DiskManager::inode_per_block * DiskManager::inode_size;
    readBlock(b_id).getData(buffer, offs, DiskManager::inode_size);
    ret.load(buffer);
    return ret;
}