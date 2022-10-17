#include "simdisk/disk-block.h"

void DiskBlock::bitset_to_ptr(std::bitset<bit_size> data, char *buffer) {
    for (int i = 0; i < byte_size; i++) {
        char ch = '\0';
        for (int j = 0; j < 8; j++) {
            if (data.test(i * 8 + j))
                ch |= (1 << (7 - j));
        }
        buffer[i] = ch;
    }
}

std::bitset<DiskBlock::bit_size> DiskBlock::ptr_to_bitset(char *buffer) {
    std::bitset<DiskBlock::bit_size> ret;
    for (int i = 0; i < byte_size; i++) {
        char ch = buffer[i];
        for (int j = 0; j < 8; j++) {
            ret.set(i * 8 + j, ch & (1 << (7 - j)));
        }
    }
    return ret;
}

int DiskBlock::getID() {
    return m_id;
}

std::bitset<DiskBlock::bit_size> DiskBlock::getData() {
    return m_data;
}