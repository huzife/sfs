#include "simsh/simsh.h"

int main() {
    // get pid of simdisk
	int simdisk_pid = -1;
	FILE *fp;
	char buffer[256];
	if ((fp = popen("pidof simdisk", "r")) != nullptr) {
		if (fgets(buffer, sizeof(buffer), fp) != nullptr) {
			simdisk_pid = strtol(buffer, nullptr, 10);
		}
	}
    pclose(fp);

	// return if couldn't find simdisk
	if (simdisk_pid == -1) {
		std::cerr << "couldn't find simdisk" << std::endl;
		return -1;
	}

    SimShell sh(simdisk_pid, getpid());
    sh.start();

    return 0;
}