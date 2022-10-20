#ifndef __BASIC_H
#define __BASIC_H

#include <memory>
#include <bitset>
#include <string>
#include <vector>
#include <cstdlib>
#include <chrono>
#include <string.h>
#include <assert.h>
#include <iostream>               // for debug

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

// TODO: modify defination of SuperBlock
// define super block
class SuperBlock : public Data {
public:
    static constexpr int max_size = 200;

    int m_count;
    std::vector<int> m_free_block;

    std::shared_ptr<char[]> dump() override;

    void load(std::shared_ptr<char[]> buffer, int size) override;
};

// define block allocation map
template <size_t N>
class AllocMap : public Data {
public:
    std::bitset<N> m_map;

    // get the size that this map takes up in disk, which is a multiple of size of a block
    int getSize() {
        int size = sizeof(m_map);
        if (size % DiskBlock::byte_size == 0)
            return size;
        return (size / DiskBlock::byte_size + 1) * DiskBlock::byte_size;
    }

    std::shared_ptr<char[]> dump() override {
        int size = getSize();
        std::shared_ptr<char[]> ret(new char[size]);
        memcpy(ret.get(), &m_map, size);
        return ret;
    }

    void load(std::shared_ptr<char[]> buffer, int size) override {
        memcpy(&m_map, buffer.get(), sizeof(m_map));
    }
};

// there are 7 file types in Linux
enum class FileType {
    NORMAL,    // normal file
    DIRECTORY, // directory
    BLOCK,     // block device
    CHARACTER, // character device
    LINK,      // symbolic link
    PIPE,      // pipe file
    SOCKET     // socket file
};

// define index node
class IndexNode : public Data {
private:
    int m_owner;                            // owner
    char m_permission;                      // permission
    char m_type;                            // file type
    int m_size;                             // file size(Byte)
    int m_location;                         // file location on disk
    int m_count;                            // hard link count
    std::chrono::nanoseconds m_create_time; // file create time
    std::chrono::nanoseconds m_access_time; // last access time
    std::chrono::nanoseconds m_modify_time; // last modify time
    std::chrono::nanoseconds m_change_time; // last change time

    void test() {
        sizeof(IndexNode);
        sizeof(FileType);
    }

    std::shared_ptr<char[]> dump() override;

    void load(std::shared_ptr<char[]> buffer, int size) override;
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