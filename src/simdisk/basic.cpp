#include "simdisk/basic.h"
#include "simdisk/disk-manager.h" // for some constant
#include <iostream>               // for debug

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
std::shared_ptr<char[]> FAT::dump() {
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
std::shared_ptr<char[]> SuperBlock::dump() {
    std::shared_ptr<char[]> ret(new char[DiskManager::block_size]);
    memcpy(ret.get(), &m_count, sizeof(m_count));
    memcpy(ret.get() + sizeof(m_count), m_free_block.data(), m_free_block.size() * 4);

    return ret;
}

void SuperBlock::load(std::shared_ptr<char[]> buffer, int size) {
    assert(size % DiskManager::block_size == 0);
    m_free_block.resize(max_size);
    memcpy(&m_count, buffer.get(), sizeof(m_count));
    memcpy(m_free_block.data(), buffer.get() + sizeof(m_count), max_size * 4);
}