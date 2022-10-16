#ifndef __DISK_BLOCK_H
#define __DISK_BLOCK_H

#include <bitset>

class DiskBlock {
public:
    static constexpr int byte_size = 1024;  // 1 KiB per block
    static constexpr int bit_size = 8 * byte_size;

public:
    int m_id;
    std::bitset<bit_size> m_data;

public:
    DiskBlock() : m_id(-1), m_data(std::bitset<bit_size>()) {}
    
    DiskBlock(int id) : m_id(id), m_data(std::bitset<bit_size>()) {}

    DiskBlock(int id, char *data) : m_id(id), m_data(data) {}

};

#endif  // __DISK_BLOCK_H