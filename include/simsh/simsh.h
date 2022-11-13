#ifndef __SIMSH_H
#define __SIMSH_H

#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <assert.h>
#include <memory>
#include <unordered_map>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include "simdisk/communication.h"

class SimShell {
public:
	int m_histsize;									// the max size of recorded commands
	std::vector<std::string> m_history;				// recording history commands
	std::unordered_map<std::string, int> m_builtin; // simshell built-in commands

private:
	const int simdisk_id; // pid of simdisk
	const int m_pid;	  // pid, also the key of share memory
	int m_shm_id;
	ShareMemory *m_shm;

public:
	SimShell(int id, int pid) : m_history(1000), simdisk_id(id), m_pid(pid) {}

	void start();

	void send(std::string command);

	void run();
};

#endif // __SIMSH_H