#include "simsh/simsh.h"

void SimShell::start() {
	// send connecting request
	Requset req;
	int qid = msgget(simdisk_id, IPC_CREAT | 0666);

	req.type = Requset::req_type;
	req.pid = m_pid;
	std::cout << "login: ";
	std::cin >> req.uid;
	std::cin.ignore();

	msgsnd(qid, &req, Requset::req_size, 0);
	m_shm_id = shmget(m_pid, sizeof(ShareMemory), IPC_CREAT | 0777);
	if (m_shm_id == -1) {
		std::cerr << "failed to get share memory" << std::endl;
		return;
	}
	m_shm = (ShareMemory *)(shmat(m_shm_id, nullptr, 0));

	run();
}

void SimShell::send(std::string command) {
	// TODO: send command to simdisk
	strcpy(m_shm->buffer, command.data());
	m_shm->size = command.size();

	// block until simdisk received successfully
	while (m_shm->size > 0) {}
	if (m_shm->size != 0) {
		std::string ret = m_shm->buffer;
		std::cout << ret << std::endl;
	}
}

void SimShell::run() {
	std::string command;
	std::cout << "simsh> ";

	while (std::getline(std::cin, command)) {
		if (command == "exit")
			break;
		send(command);
		std::cout << "simsh> ";
	}
	std::cout << "logout" << std::endl;
}
