#include "simdisk/disk-manager.h"

std::shared_ptr<DiskManager> DiskManager::instance;

DiskManager::DiskManager() : m_fat(DiskManager::fat_block_count) {
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

void DiskManager::writeBlock(int id, DiskBlock &block) {
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

int DiskManager::expandSize(int size) {
    int cnt = size / DiskManager::block_size;
    if (size % DiskManager::block_size)
        cnt++;

    return cnt * DiskManager::block_size;
}

std::string DiskManager::timeToDate(const std::chrono::system_clock::time_point &time) {
    time_t t = std::chrono::system_clock::to_time_t(time);
    tm p;
    char buffer[256];
    localtime_r(&t, &p);
    strftime(buffer, 256, "%Y-%m-%d %H:%M:%S", &p);

    return std::string(buffer);
}

std::shared_ptr<IndexNode> DiskManager::getIndexNode(int id) {
    assert(id < m_super_block.m_total_inode);
    auto ret = std::make_shared<IndexNode>();
    std::shared_ptr<char[]> buffer(new char[DiskManager::inode_size]);
    int b_id = id / DiskManager::inode_per_block + DiskManager::inode_block_offset;
    int offs = id % DiskManager::inode_per_block * DiskManager::inode_size;
    readBlock(b_id).getData(buffer, offs, DiskManager::inode_size);
    ret->load(buffer);

    return ret;
}

std::shared_ptr<File> DiskManager::getFile(std::shared_ptr<IndexNode> inode) {
    std::shared_ptr<File> ret;
    int size = inode->m_size;
    int subs = inode->m_subs;

    if (inode->m_type == FileType::DIRECTORY) {
        ret = std::make_shared<DirFile>(size, subs);
    }
    else {
        size = expandSize(size);
        ret = std::make_shared<DataFile>(size);
    }

    std::shared_ptr<char[]> buffer(new char[size]);
    int i = 0;
    int cur = inode->m_location;
    while (cur != -1) {
        readBlock(cur + DiskManager::file_block_offset).getData(buffer, i * DiskManager::block_size);
        i++;
        cur = m_fat[cur];
    }
    ret->load(buffer);
    
    return ret;
}

int DiskManager::allocIndexNode() {
    for (int i = 0; i < m_inode_map.size(); i++) {
        if (!m_inode_map.test(i)) {
            m_inode_map.set(i);
            return i;
        }
    }

    return -1; // return -1 if there is no free index node
}

int DiskManager::allocFileBlock(int n) {
    std::vector<int> blocks;
    for (int i = 0; i < m_block_map.size(); i++) {
        if (!m_block_map.test(i)) {
            blocks.emplace_back(i);
            if (--n == 0) break;
        }
    }

    if (n > 0) {
        for (int j = 0; j < blocks.size() - 1; j++) {
            m_block_map.set(blocks[j]);
            m_fat[blocks[j]] = blocks[j + 1];
        }
        m_block_map.set(blocks.back());
        return blocks.front();
    }

    return -1; // return -1 if there is no enough free block
}

void DiskManager::freeIndexNode(int id) {
    m_inode_map.reset(id);
}

void DiskManager::freeFlieBlock(int id) {
    int cur = id;
    while (cur != -1) {
        int next = m_fat[cur];
        m_fat[cur] = -1;
        m_block_map.reset(cur);
        cur = next;
    }
}