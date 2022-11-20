#include "simsh/simsh.h"

void SimShell::start() {
	// send connecting request
	Requset req;
	int qid = msgget(simdisk_id, IPC_CREAT | 0666);

	req.type = Requset::req_type;
	req.pid = m_pid;
	req.uid = 111;
	std::cout << "login: ";
	std::cin >> m_user_name;
	std::cin.ignore();
	m_cwd = "/";

	msgsnd(qid, &req, Requset::req_size, 0);
	m_shm_id = shmget(m_pid, sizeof(ShareMemory), IPC_CREAT | 0777);
	if (m_shm_id == -1) {
		std::cerr << "failed to get share memory" << std::endl;
		return;
	}
	m_shm = (ShareMemory *)(shmat(m_shm_id, nullptr, 0));

	run();
}

void SimShell::exec(std::string command) {
	int l = command.find_first_not_of(' ');
	int r = command.find_last_not_of(' ');
	if (l == -1) return;

	command = command.substr(l, r - l + 1);
	if (!checkBuiltin(command)) {
		send(command);
	}
	addHistory(command);
}

bool SimShell::checkBuiltin(std::string command) {
	int r = command.find_first_of(' ');
	std::string cmd_name = command.substr(0, r);

	if (cmd_name == "clear")
		clear();
	else if (cmd_name == "exit")
		exit();
	else if (cmd_name == "history") {
		int num;
		if (r == -1) {
			num = m_histsize;
		}
		else {
			int l = r + 1;
			r = command.find_first_of(' ', l);
			num = strtol(command.substr(l, r).data(), nullptr, 10);
		}
		history(num);
	}
	else
		return false;

	return true;
}

void SimShell::send(std::string command) {
	strcpy(m_shm->buffer, command.data());
	m_shm->size = command.size();

	// block until simdisk received successfully
	while (m_shm->size > 0) {}
	if (m_shm->size != 0) {
		std::string ret = m_shm->buffer;
		if (ret.back() != '\n')
			ret += '\n';

		if (ret.substr(0, 7) == "cd ret:") {
			m_cwd = m_shm->buffer + 7;
			if (m_cwd.size() > 1 && m_cwd.back() == '/')
				m_cwd.pop_back();
			return;
		}

		std::cout << ret;
		setY(getLineCount(ret));
	}
}

void SimShell::run() {
	ioctl(STDIN_FILENO, TIOCGWINSZ, &win_size);
	x = 0;
	y = 0;

	system("clear");
	system("stty -echo");
	system("stty -icanon");

	std::string command;
	while (!m_logout) {
		showInfo();
		getInput();

		command = buffer;
		exec(command);
	}

	system("stty echo");
	system("stty icanon");

	std::cout << "logout" << std::endl;
}

int SimShell::getLineCount(std::string str) {
	if (str.empty()) return 0;

	assert(str.back() == '\n');
	std::string s = str;
	int ret = 0;
	int l = 0, r;

	while (l < str.size()) {
		r = s.find_first_of('\n', l);
		ret += (r - l - 1) / win_size.ws_col + 1;
		l = r + 1;
	}

	return ret;
}

void SimShell::setY(int inc) {
	y = std::min<int>(y + inc, win_size.ws_row - 1);
}

std::string SimShell::getColorSymbol(Color c) {
	switch (c) {
	case Color::BLACK: return "\033[01;30m";
	case Color::RED: return "\033[01;31m";
	case Color::GREEN: return "\033[01;32m";
	case Color::YELLOW: return "\033[01;33m";
	case Color::BLUE: return "\033[01;34m";
	case Color::PURPLE: return "\033[01;35m";
	case Color::AZURE: return "\033[01;36m";
	case Color::WHITE: return "\033[01;37m";
	}

	return "";
}

void SimShell::print(std::string str, Color c) {
	std::cout << getColorSymbol(c) << str << "\033[00m";
}

void SimShell::flush() {
	// clear rows
	for (int i = y + 1; i < win_size.ws_row; i++) {
		std::cout << "\033[" << i + 1 << ';' << 1 << 'H';
		std::cout << "\033[K";
	}
	std::cout << "\033[" << y + 1 << ';' << x + 1 << 'H'; // reset cursor
	std::cout << "\033[K";								  // clear from cursor to the end of line
	std::cout << buffer;								  // print buffer

	int d = (x + strlen(buffer) - 1) / win_size.ws_col;
	if (y + d >= win_size.ws_row)
		y = win_size.ws_row - d - 1;

	std::cout << "\033[" << y + 1 << ';' << x + 1 << 'H'; // reset cursor

	// positioning cursor
	std::cout << "\033[" << y + (cur_idx + x) / win_size.ws_col + 1 << ';' << (cur_idx + x) % win_size.ws_col + 1 << 'H';

	fflush(stdout);
}

void SimShell::showInfo() {
	print(m_user_name, Color::GREEN);
	print(":", Color::YELLOW);
	print(m_cwd, Color::BLUE);
	print("$ ", Color::YELLOW);

	// update x and y;
	int len = m_user_name.size() + m_cwd.size() + 3;
	x = len % win_size.ws_col;
	setY(len / win_size.ws_col);
}

void SimShell::getInput() {
	char ch;
	buffer[0] = '\0';
	cur_idx = 0;
	hist_idx = m_history.size();

	while (1) {
		bool end = false;
		ch = getchar();
		if ('\033' == ch && getchar() == '[') // arrow key
		{
			switch (ch = getchar()) {
			case 'A': lastCommand(); break; // up
			case 'B': nextCommand(); break; // down
			case 'C': moveRight(); break;	// right
			case 'D': moveLeft(); break;	// left
			}
		}
		else {
			switch (ch) {
			case 4: strcpy(buffer, "exit"); return; // ctrl+d
			case 127: deleteChar(); break;			// backspace
			case '\n': end = true; break;			// enter
			default: insertChar(ch); break;
			}
		}

		if (end) break;

		flush();
	}

	std::cout << std::endl;

	// update x and y
	int len = strlen(buffer) + x;
	x = 0;
	setY((len - 1) / win_size.ws_col + 1);
}

void SimShell::moveLeft() {
	if (cur_idx > 0) cur_idx--;
}

void SimShell::moveRight() {
	if (cur_idx < strlen(buffer)) cur_idx++;
}

void SimShell::lastCommand() {
	if (hist_idx <= 0) return;

	strcpy(buffer, m_history[--hist_idx].data());
	cur_idx = strlen(buffer);
}

void SimShell::nextCommand() {
	if (hist_idx >= m_history.size() - 1) return;

	strcpy(buffer, m_history[++hist_idx].data());
	cur_idx = strlen(buffer);
}

void SimShell::deleteChar() {
	if (cur_idx > 0) {
		memmove(buffer + cur_idx - 1, buffer + cur_idx, strlen(buffer) - cur_idx + 1);
		cur_idx--;
	}
}

void SimShell::insertChar(char ch) {
	if (strlen(buffer) < 4095) {
		memmove(buffer + cur_idx + 1, buffer + cur_idx, strlen(buffer) - cur_idx + 1);
		buffer[cur_idx] = ch;
		cur_idx++;
	}
}

void SimShell::addHistory(std::string command) {
	if (!m_history.empty() && m_history.back() == command)
		return;

	m_history.emplace_back(command);
	if (m_history.size() > m_histsize)
		m_history.erase(m_history.begin());
}
