#include <iostream>
#include <signal.h>
#include "simdisk/initializor.h"
#include "simdisk/disk-manager.h"

int main(int argc, char *argv[]) {
	// ctrl+c to shutdown
	// signal(SIGINT, [](int sig) {
	// std::cout << "sigint" << std::endl;
	// int qid = msgget(getpid(), IPC_CREAT | 0666);
	// Requset s;
	// s.type = Requset::req_type;
	// s.pid = -1;
	// msgsnd(qid, &s, Requset::req_size, 0);
	// });
	signal(SIGINT, [](int) {});
	system("clear");
	auto disk_manager = std::make_shared<DiskManager>();
	disk_manager->start();

	return 0;
}