#include <iostream>
#include "simdisk/initializor.h"
#include "simdisk/disk-manager.h"

void DiskManager::test() {
	std::string command;
	std::cout << "simdisk> ";
	while (std::getline(std::cin, command)) {
		if (command == "exit")
			break;
		exec(command);
		std::cout << "simdisk> ";
	}
}

int f();

int main(int argc, char *argv[]) {
	system("clear");
	auto disk_manager = std::make_shared<DiskManager>();
	disk_manager->start();

	disk_manager->test();
	return 0;
}