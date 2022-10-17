#ifndef __BASIC_H
#define __BASIC_H

#include <memory>
#include <bitset>
#include <string>
#include <vector>
#include <any>

class FreeGroup;

// define a disk block
class DiskBlock {
public:
    static constexpr int byte_size = 1024; // 1 KiB per block
    static constexpr int bit_size = 8 * byte_size;

private:
    int m_id;
    std::bitset<bit_size> m_data;

public:
    DiskBlock() : m_id(-1), m_data(std::bitset<bit_size>()) {}

    DiskBlock(int id) : m_id(id), m_data(std::bitset<bit_size>()) {}

    DiskBlock(int id, std::shared_ptr<char> buffer) : m_id(id), m_data(chptr_to_bitset(buffer, DiskBlock::byte_size)) {}

    static void bitset_to_chptr(std::bitset<bit_size> data, std::shared_ptr<char> buffer, int size);

    static std::bitset<bit_size> chptr_to_bitset(std::shared_ptr<char> buffer, int size);

    int getID();

    std::bitset<bit_size> getData();

    template <class T>
    void setData(std::shared_ptr<T> data) {
        m_data = chptr_to_bitset(std::reinterpret_pointer_cast<char>(data), sizeof(T));
    }
};

// define file allocation table
class FAT {
private:
    std::vector<int> m_table;
};

// define free group
class FreeGroup {
public:
    static constexpr int max_size = 200;

    int m_count;
    int m_free_block[max_size];
};

// define index node
class IndexNode {
private:
    int m_id;
    int m_size;
    int m_location;
    char m_permission;

    int m_count;
};

// define directory entry(fcb)
class DirectoryEntry {
private:
    std::string m_filename;
    int m_inode;
};

// define file
class File {
public:
};

class DataFile : public File {
};

class DirFile : public File {
private:
    DirectoryEntry m_parent;
    DirectoryEntry m_current;
    std::vector<DirectoryEntry> m_dirs;
};

#endif // __BASIC_H