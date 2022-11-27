#ifndef __SIMSH_H
#define __SIMSH_H

#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <assert.h>
#include <memory>
#include <unordered_map>
#include <algorithm>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/ioctl.h>
#include "simdisk/communication.h"

class SimShell {
public:
	enum class Color {
		BLACK,
		RED,
		GREEN,
		YELLOW,
		BLUE,
		PURPLE,
		AZURE,
		WHITE
	};

	int m_histsize;									// the max size of recorded commands
	std::vector<std::string> m_history;				// recording history commands
	std::unordered_map<std::string, int> m_builtin; // simshell built-in commands

private:
	const int simdisk_id; // pid of simdisk
	const int m_pid;	  // pid, also the key of share memory
	int m_shm_id;
	bool m_logout;
	ShareMemory *m_shm;

	std::string m_user_name;
	std::string m_home;
	std::string m_cwd;

	// terminal
	winsize win_size;  // terminal size
	int x, y;		   // cursor position
	char buffer[4096]; // input buffer
	int cur_idx;	   // index of cursor in command
	int hist_idx;	   // index of history command

public:
	SimShell(int id, int pid) : m_histsize(1000), simdisk_id(id), m_pid(pid), m_logout(false) {}

	void start();

	void test();

private:
	void exec(std::string command);

	bool checkBuiltin(std::string command);

	bool checkOutput();

	void send(std::string command);

	void run();

	int getLineCount(std::string str);

	void setY(int inc);

	std::string getColorSymbol(Color c);

	void print(std::string str, Color c = Color::WHITE);

	void flush();

	void showInfo();

	void getInput();

	void moveRight();

	void moveLeft();

	void moveFront();

	void moveBack();

	void lastCommand();

	void nextCommand();

	void deleteChar();

	void insertChar(char ch);

	void addHistory(std::string command);

	// built-in
	int clear();
	int history(int num);
	int exit();
};

#endif // __SIMSH_H