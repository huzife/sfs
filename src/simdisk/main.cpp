#include <iostream>
#include "simdisk/initializor.h"
#include "simdisk/disk-manager.h"

int main(int argc, char *argv[]) {
    system("clear");
    auto disk_manager = DiskManager::getInstance();
    disk_manager->initDisk();

    return 0;
}