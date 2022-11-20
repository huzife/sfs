#include "simsh/simsh.h"

int SimShell::clear() {
	std::cout << "\033[2J\033[1;1H";
	x = 0;
	y = 0;
	fflush(stdout);

	return 0;
}

int SimShell::history(int num) {
	std::string out;
	if (num <= 0) {
		out = "history: invalid argument:" + std::to_string(num) + "\n";
	}
	else {
		num = std::min<int>(num, m_history.size());
		int i = m_history.size() - num;
		while (i < m_history.size()) {
			out += std::to_string(i + 1) + ": " + m_history[i] + "\n";
            i++;
		}
	}

	std::cout << out;
	setY(getLineCount(out));

    return 0;
}

int SimShell::exit() {
    strcpy(m_shm->buffer, "exit");
    m_shm->size = strlen("exit");
    m_logout = true;

    return 0;
}