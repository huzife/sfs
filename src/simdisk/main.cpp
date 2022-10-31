#include <iostream>
#include "simdisk/initializor.h"
#include "simdisk/disk-manager.h"

void DiskManager::test() {
	// char *argv[2] = {"dir", "-s"};
	// dir(2, argv);
	info();
}

int main(int argc, char *argv[]) {
	system("clear");
	auto disk_manager = DiskManager::getInstance();

	disk_manager->test();

	return 0;
}