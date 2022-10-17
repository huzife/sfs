#ifndef __DISK_BLOCK_H
#define __DISK_BLOCK_H

#include <bitset>
#include <any>

class DiskBlock {
public:
    static constexpr int byte_size = 1024;  // 1 KiB per block
    static constexpr int bit_size = 8 * byte_size;

private:
    int m_id;
    std::bitset<bit_size> m_data;

public:
    DiskBlock() : m_id(-1), m_data(std::bitset<bit_size>()) {}
    
    DiskBlock(int id) : m_id(id), m_data(std::bitset<bit_size>()) {}

    DiskBlock(int id, char *buffer) : m_id(id), m_data(ptr_to_bitset(buffer)) {}

    static void bitset_to_ptr(std::bitset<bit_size> data, char *buffer);

    static std::bitset<bit_size> ptr_to_bitset(char *buffer);

    int getID();

    std::bitset<bit_size> getData();
};

#endif  // __DISK_BLOCK_H