#include "simdisk/basic.h"
#include "simdisk/disk-manager.h" // for some constant

int DiskBlock::getID() {
    return m_id;
}

std::bitset<DiskBlock::bit_size> DiskBlock::getBitData() {
    return m_data;
}

void DiskBlock::getData(std::shared_ptr<char[]> data, int offset) {
    for (int i = 0; i < byte_size; i++) {
        char ch = 0;
        for (int j = 0; j < 8; j++) {
            if (m_data.test(i * 8 + j))
                ch |= (1 << (7 - j));
        }
        data[i + offset] = ch;
    }
}

void DiskBlock::setData(std::shared_ptr<char[]> data, int offset) {
    for (int i = 0; i < byte_size; i++) {
        // char ch = buffer.get()[i + offset];
        char ch = data[i + offset];
        for (int j = 0; j < 8; j++) {
            m_data.set(i * 8 + j, ch & (1 << (7 - j)));
        }
    }
}

// FAT
std::shared_ptr<char[]> FAT::dump(int size) {
    std::shared_ptr<char[]> ret(new char[m_table.size() * 4]);
    memcpy(ret.get(), m_table.data(), m_table.size() * 4);

    return ret;
}

void FAT::load(std::shared_ptr<char[]> buffer, int size) {
    assert(size % DiskManager::block_size == 0);
    m_table.resize(DiskManager::block_count);
    memcpy(m_table.data(), buffer.get(), DiskManager::fat_size);
}

// SuperBlock
std::shared_ptr<char[]> SuperBlock::dump(int size) {
    std::shared_ptr<char[]> ret(new char[DiskManager::block_size]);
    memcpy(ret.get(), (char *)this + sizeof(Data), 40);
    memcpy(ret.get() + 40, m_root.dump().get(), m_root.m_rec_len);

    return ret;
}

void SuperBlock::load(std::shared_ptr<char[]> buffer, int size) {
    memcpy((char *)this + sizeof(Data), buffer.get(), 40);
    int16_t root_size = *(buffer.get() + 44);
    std::shared_ptr<char[]> temp(new char[DiskManager::block_size]);
    memcpy(temp.get(), buffer.get() + 40, root_size);
    m_root.load(temp, DiskManager::block_size);
    m_dirt = false;
}

// IndexNode
std::shared_ptr<char[]> IndexNode::dump(int size) {
    std::shared_ptr<char[]> ret(new char[DiskManager::block_size]);
    memcpy(ret.get(), (char *)this + sizeof(Data), sizeof(IndexNode) - sizeof(Data));

    return ret;
}

void IndexNode::load(std::shared_ptr<char[]> buffer, int size) {
    memcpy((char *)this + sizeof(Data), buffer.get(), sizeof(IndexNode) - sizeof(Data));
}

// DirectoryEntry
std::shared_ptr<char[]> DirectoryEntry::dump(int size) {
    std::shared_ptr<char[]> ret(new char[DiskManager::block_size]);
    memcpy(ret.get(), (char *)this + sizeof(Data), 7);
    memcpy(ret.get() + 7, m_filename.data(), m_name_len);

    return ret;
}

void DirectoryEntry::load(std::shared_ptr<char[]> buffer, int size) {
    memcpy((char *)this + sizeof(Data), buffer.get(), 7);
    m_filename.assign(buffer.get() + 7, m_name_len);
}

// DirFile
std::shared_ptr<char[]> DirFile::dump(int size) {
    std::shared_ptr<char[]> ret(new char[size]);
    int offset = 0;
    memcpy(ret.get(), m_parent.dump().get(), m_parent.m_rec_len);
    offset += m_parent.m_rec_len;
    memcpy(ret.get() + offset, m_current.dump().get(), m_current.m_rec_len);
    offset += m_current.m_rec_len;
    for (auto d : m_dirs) {
        memcpy(ret.get() + offset, d.dump().get(), d.m_rec_len);
        offset += d.m_rec_len;
    }

    return ret;
}

void DirFile::load(std::shared_ptr<char[]> buffer, int size) {
    int offset = 0;
    int16_t len;
    std::shared_ptr<char[]> temp(new char[DiskManager::block_size]);
    // m_parent
    len = *(buffer.get() + offset + 4);
    memcpy(temp.get(), buffer.get() + offset, len);
    m_parent.load(temp, len);
    offset += len;
    // m_current
    len = *(buffer.get() + offset + 4);
    memcpy(temp.get(), buffer.get() + offset, len);
    m_current.load(temp, len);
    offset += len;

    // m_dirs
    while (offset < size) {
        len = *(buffer.get() + offset + 4);
        memcpy(temp.get(), buffer.get() + offset, len);
        DirectoryEntry d;
        d.load(temp, len);
        m_dirs.emplace_back(d);
        offset += len;
    }
}