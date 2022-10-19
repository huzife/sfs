#ifndef __INITIALIZOR_H
#define __INITIALIZOR_H

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include "simdisk/basic.h"

class DiskManager;

class Initializor {
private:
    std::string m_path;
    const int m_block_size;
    const int m_block_count;

public:
    // Initializor();

    Initializor(std::string path, int block_size, int block_count)
        : m_path(path), m_block_size(block_size), m_block_count(block_count) {}

    void setWriteBlock(void (*writeBlock)(int, DiskBlock));

    void init(); // initialized file system

private:
    bool exist(); // check if the file system is exists

    void create(); // create file system

    void format(); // format the disk
};

#endif // __INITIALIZOR_H