#include <iostream>
#include "simdisk/initializor.h"
#include "simdisk/disk-manager.h"

void DiskManager::test() {
	char *argv[2] = {"cd", "."};
	cd(2, argv);
}

int main(int argc, char *argv[]) {
	system("clear");
	auto disk_manager = DiskManager::getInstance();

	disk_manager->test();

	return 0;
}