#include <iostream>
#include "simdisk/initializor.h"
#include "simdisk/disk-manager.h"

void DiskManager::test() {
	std::string command;
	while (!std::cin.eof()) {
		std::cout << "simdisk> ";
		std::getline(std::cin, command);
		if (command == "exit") break;
		exec(command, 0);
	}
}

int f();

int main(int argc, char *argv[]) {
	system("clear");
	auto disk_manager = std::make_shared<DiskManager>();
	disk_manager->start();

	return 0;
}