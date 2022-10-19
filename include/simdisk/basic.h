#ifndef __BASIC_H
#define __BASIC_H

#include <memory>
#include <bitset>
#include <string>
#include <vector>
#include <cstdlib>
#include <string.h>
#include <assert.h>

class DiskManager;

// define a disk block
class DiskBlock {
public:
    static constexpr int byte_size = 1024;         // 1 KiB per block
    static constexpr int bit_size = byte_size * 8; // 8 Kib per block
private:
    int m_id;
    std::bitset<bit_size> m_data;

public:
    DiskBlock() : m_id(-1), m_data(std::bitset<bit_size>()) {}

    DiskBlock(int id) : m_id(id), m_data(std::bitset<bit_size>()) {}

    int getID();

    std::bitset<bit_size> getBitData();

    void getData(std::shared_ptr<char[]> data, int offset = 0);

    void setData(std::shared_ptr<char[]> data, int offset = 0);
};

// define a Data type to implement binary data saving and loading
class Data {
public:
    virtual std::shared_ptr<char[]> dump() = 0;

    virtual void load(std::shared_ptr<char[]> buffer, int size) = 0;
};

// define file allocation table
class FAT : public Data {
public:
    std::vector<int> m_table;

    std::shared_ptr<char[]> dump() override;

    void load(std::shared_ptr<char[]> buffer, int size) override;
};

// define free group
class SuperBlock : public Data {
public:
    static constexpr int max_size = 200;

    int m_count;
    std::vector<int> m_free_block;

    std::shared_ptr<char[]> dump() override;

    void load(std::shared_ptr<char[]> buffer, int size) override;
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