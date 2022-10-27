#include <iostream>
#include "simdisk/initializor.h"
#include "simdisk/disk-manager.h"

void DiskManager::test() {
    std::string path = "/root/sfs/build/bin/simdisk";
    auto dirs = splitPath(path);
    for (auto s : dirs) {
        std::cout << s << std::endl;
    }
}

int main(int argc, char *argv[]) {
    system("clear");
    auto disk_manager = DiskManager::getInstance();
    disk_manager->initDisk();
    disk_manager->boot();

    disk_manager->test();

    return 0;
}