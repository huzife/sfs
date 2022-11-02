#ifndef __DISK_MANAGER_H
#define __DISK_MANAGER_H

#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <unistd.h>
#include <memory>
#include <string>
#include <assert.h>
#include <functional>
#include <algorithm>
#include "simdisk/initializor.h"
#include "simdisk/basic.h"

class DiskManager : public std::enable_shared_from_this<DiskManager> {
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
    static constexpr int super_block_id = 0;                        // super block is 0
    static constexpr int inode_size = 64;                           // 64 Byte per index node
    static constexpr int inode_per_block = block_size / inode_size; // 16 inode per block

    using cfp = int (DiskManager::*)(int, char *[]);
    // typedef int (DiskManager::*cfp)(int, char **);

private:
    static std::shared_ptr<DiskManager> instance;

    bool m_is_on;

    std::string m_disk_path;

    SuperBlock m_super_block;
    FAT m_fat;
    AllocMap<file_block_count> m_inode_map;
    AllocMap<file_block_count> m_block_map;
    CWD m_cwd;

public:
    DiskManager();

    ~DiskManager();

    void start();

    int expandedSize(int size);

    // this is used for testing
    void test();

private:
    DiskBlock readBlock(int id);

    void writeBlock(int id, DiskBlock &block);

    void initDisk();

    void boot();

    void shutdown();

    void loadSuperBlock();

    void loadFAT();

    void loadINodeMap();

    void loadBlockMap();

    void saveSuperBlock();

    void saveFAT();

    void saveINodeMap();

    void saveBlockMap();

    void initCWD();

    std::string timeToDate(const std::chrono::system_clock::time_point &time);

    std::shared_ptr<IndexNode> getIndexNode(int id);

    void writeIndexNode(int id, std::shared_ptr<IndexNode> inode);

    std::shared_ptr<DirectoryEntry> getDirectoryEntry(std::string path);

    std::shared_ptr<File> getFile(std::shared_ptr<IndexNode> inode);

    void writeFile(std::shared_ptr<IndexNode> inode, std::shared_ptr<File> file);

    int getDirSize(std::shared_ptr<IndexNode> inode);

    int expandFileSize(std::shared_ptr<IndexNode> inode, int size);

    int getLastBlock(int id);

    int allocIndexNode();

    int allocFileBlock(int n);

    void expandBlock(int id, int n);

    void freeIndexNode(int id);

    void freeFlieBlock(int id);

    int exec(std::string command);

    cfp getFuncPtr(std::string command_name);

    std::vector<std::string> splitArgs(std::string command);

    std::vector<std::string> splitPath(std::string path);

	std::string getPath(std::string cur, std::string path);

    // commands
    int info(int argc = 0, char *argv[] = nullptr);
    int cd(int argc, char *argv[]);
    int dir(int argc, char *argv[]);
    int md(int argc, char *argv[]);
    int rd(int argc, char *argv[]);
    int newfile(int argc, char *argv[]);
    int cat(int argc, char *argv[]);
    int copy(int argc, char *argv[]);
    int del(int argc, char *argv[]);
    int check(int argc, char *argv[]);
};

#endif // __DISK_MANAGER_H