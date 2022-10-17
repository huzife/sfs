#ifndef __DISK_MANAGER_H
#define __DISK_MANAGER_H

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <memory>
#include <string>
#include <assert.h>
#include "simdisk/initializor.h"
#include "simdisk/disk-block.h"


// there can only be one disk manager, so I use singleton mode
class DiskManager {
    // make sure std::make_unique has enough rights to call DiskManager()
    friend std::unique_ptr<DiskManager> std::make_unique<DiskManager>();

private:
    static std::unique_ptr<DiskManager> instance;

    std::string m_disk_path;
    std::unique_ptr<Initializor> m_initializor;

public:
    static std::unique_ptr<DiskManager>& getInstance();

    void initDisk();

    DiskBlock readBlock(int id);

    void writeBlock(int id, DiskBlock block);

protected:
    DiskManager();

};

#endif  // __DISK_MANAGER_H