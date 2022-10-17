#include "simdisk/basic.h"

void DiskBlock::bitset_to_chptr(std::bitset<bit_size> data, std::shared_ptr<char> buffer, int size) {
    auto chptr = buffer.get();
    for (int i = 0; i < size; i++) {
        char ch = 0;
        for (int j = 0; j < 8; j++) {
            if (data.test(i * 8 + j))
                ch |= (1 << (7 - j));
        }
        chptr[i] = ch;
    }

}

std::bitset<DiskBlock::bit_size> DiskBlock::chptr_to_bitset(std::shared_ptr<char> buffer, int size) {
    std::bitset<DiskBlock::bit_size> ret;
    for (int i = 0; i < size; i++) {
        char ch = buffer.get()[i];
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