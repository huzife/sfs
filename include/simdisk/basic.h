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
#include <iostream> // for debug

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

    void getData(std::shared_ptr<char[]> data, int size = byte_size, int offset1 = 0, int offset2 = 0);

    void setData(std::shared_ptr<char[]> data, int size = byte_size, int offset1 = 0, int offset2 = 0);
};

// define a Data type to implement binary data saving and loading
class Data {
public:
    virtual std::shared_ptr<char[]> dump(int size) = 0;

    virtual void load(std::shared_ptr<char[]> buffer, int size) = 0;
};

// define file allocation table
class FAT : public Data {
public:
    std::vector<int> m_table;

    std::shared_ptr<char[]> dump(int size = 0) override;

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

    // overload some function provide by bitset
    void set() { m_map.set(); }
    void set(size_t pos, bool val = true) { m_map.set(N - pos - 1, val); }
    void reset() { m_map.reset(); }
    void reset(size_t pos) { m_map.reset(N - pos - 1); }
    bool test(size_t pos) { return m_map.test(N - pos - 1); }
    std::bitset<N>::reference operator[](size_t pos) { return m_map[N - pos - 1]; }

    std::shared_ptr<char[]> dump(int size = 0) override {
        int s = getSize();
        std::shared_ptr<char[]> ret(new char[s]);
        memcpy(ret.get(), &m_map, s);
        return ret;
    }

    void load(std::shared_ptr<char[]> buffer, int size) override {
        memcpy(&m_map, buffer.get(), sizeof(m_map));
    }
};

// there are 7 file types in Linux
enum class FileType : int8_t {
    NORMAL,    // normal file
    DIRECTORY, // directory
    BLOCK,     // block device
    CHARACTER, // character device
    LINK,      // symbolic link
    PIPE,      // pipe file
    SOCKET     // socket file
};

enum class Permission : int8_t {
    READ = 1,
    WRITE = 2,
    EXEC = 4
};

// define index node
class IndexNode : public Data {
public:
    FileType m_type;                                     // file type
    Permission m_owner_permission;                       // owner permission
    Permission m_other_permission;                       // other permission
    int16_t m_owner;                                     // owner
    int m_size;                                          // file size(Byte)
    int m_location;                                      // file location on disk
    int m_count;                                         // hard link count
    int m_subs;                                          // number of sub directories
    std::chrono::system_clock::time_point m_create_time; // file create time
    std::chrono::system_clock::time_point m_access_time; // last access time
    std::chrono::system_clock::time_point m_modify_time; // last modify time
    std::chrono::system_clock::time_point m_change_time; // last change time

    std::shared_ptr<char[]> dump(int size = 0) override;

    void load(std::shared_ptr<char[]> buffer, int size) override;
};

// define directory entry(fcb)
class DirectoryEntry : Data {
public:
    int m_inode;
    int16_t m_rec_len;
    int8_t m_name_len;
    std::string m_filename;

    std::shared_ptr<char[]> dump(int size = 0) override;

    void load(std::shared_ptr<char[]> buffer, int size) override;
};

// TODO: modify defination of SuperBlock
// define super block
class SuperBlock : public Data {
public:
    int m_fat_location;       // FAT location on disk
    int m_fat_size;           // FAT size(blocks)
    int m_block_map_location; // block map location on disk
    int m_block_map_size;     // block map size(blocks)
    int m_inode_map_location; // inode map location on disk
    int m_inode_map_size;     // inode map size(blocks)

    int m_block_size;        // size per block(Byte)
    int m_filename_maxbytes; // max length of filename(Byte)
    int m_free_block;        // free blocks left
    int m_free_inode;        // free inodes left

    DirectoryEntry m_root; // dEntry of root path(/)

    bool m_dirt; // record if the super block has been modified

    std::shared_ptr<char[]> dump(int size = 0) override;

    void load(std::shared_ptr<char[]> buffer, int size) override;
};

// define file
class File : public Data {
public:
};

class DataFile : public File {
};

class DirFile : public File {
public:
    DirectoryEntry m_parent;
    DirectoryEntry m_current;
    std::vector<DirectoryEntry> m_dirs;

    std::shared_ptr<char[]> dump(int size) override;

    void load(std::shared_ptr<char[]> buffer, int size) override;
};

#endif // __BASIC_H