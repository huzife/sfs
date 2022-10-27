#ifndef __DISK_MANAGER_H
#define __DISK_MANAGER_H

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <memory>
#include <string>
#include <assert.h>
#include <functional>
#include "simdisk/initializor.h"
#include "simdisk/basic.h"

// there can only be one disk manager, so I use singleton mode
class DiskManager {
    friend class Initializor;
public:
    // define the constants use in building the file system
    static constexpr int disk_size = 100 * 1024 * 1024;             // 100 MiB space
    static constexpr int block_size = 1024;                         // 1 KiB per block
    static constexpr int block_count = disk_size / block_size;      // 102400 blocks
    static constexpr int inode_block_count = 5999;                  // 5999 blocks for index node
    static constexpr int file_block_count = 95976;                  // 95976 blocks for file
    static constexpr int inode_block_offset = 425;                  // first inode block id is 425
    static constexpr int file_block_offset = 6424;                  // first file block id is 6424
    static constexpr int fat_size = block_count * 4;                // FAT takes up 409600 Bytes
    static constexpr int fat_block_count = fat_size / block_size;   // FAT takes up 400 blocks
    static constexpr int super_block_id = 0;                        // super block is 400
    static constexpr int inode_size = 64;                           // 64 Byte per index node
    static constexpr int inode_per_block = block_size / inode_size; // 16 inode per block

private:
    static std::shared_ptr<DiskManager> instance;

    std::string m_disk_path;
    std::unique_ptr<Initializor> m_initializor;

    SuperBlock m_super_block;
    FAT m_fat;
    AllocMap<file_block_count> m_inode_map;
    AllocMap<file_block_count> m_block_map;

public:
    static std::shared_ptr<DiskManager> getInstance();

    void initDisk();

    void boot();

    // this is used for testing
    void test();

private:
    DiskBlock readBlock(int id);

    void writeBlock(int id, DiskBlock &block);

    void loadFAT();

    void loadSuperBlock();

    void loadINodeMap();

    void loadBlockMap();

    int expandSize(int size);

    std::string timeToDate(const std::chrono::system_clock::time_point &time);

    std::shared_ptr<IndexNode> getIndexNode(int id);

    std::shared_ptr<File> getFile(std::shared_ptr<IndexNode> inode);

    int allocIndexNode();

    int allocFileBlock(int n);

    void freeIndexNode(int id);

    void freeFlieBlock(int id);

    std::vector<std::string> splitPath(std::string path);

private:
    DiskManager();
};

#endif // __DISK_MANAGER_H