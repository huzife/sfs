#ifndef __DISK_MANAGER_H
#define __DISK_MANAGER_H

#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <unistd.h>
#include <memory>
#include <string>
#include <assert.h>
#include <getopt.h>
#include <functional>
#include <atomic>
#include <algorithm>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include "simdisk/initializor.h"
#include "simdisk/basic.h"
#include "simdisk/communication.h"

class DiskManager : public std::enable_shared_from_this<DiskManager> {
	friend class Initializor;

public:
	// define the constants use in building the file system
	static constexpr int disk_size = 100 * 1024 * 1024;				// 100 MiB space
	static constexpr int block_size = 1024;							// 1 KiB per block
	static constexpr int block_count = disk_size / block_size;		// 102400 blocks
	static constexpr int inode_block_count = 5999;					// 5999 blocks for index node
	static constexpr int file_block_count = 95976;					// 95976 blocks for file
	static constexpr int inode_block_offset = 425;					// first inode block id is 425
	static constexpr int file_block_offset = 6424;					// first file block id is 6424
	static constexpr int fat_size = block_count * 4;				// FAT takes up 409600 Bytes
	static constexpr int fat_block_count = fat_size / block_size;	// FAT takes up 400 blocks
	static constexpr int super_block_id = 0;						// super block is 0
	static constexpr int inode_size = 64;							// 64 Byte per index node
	static constexpr int inode_per_block = block_size / inode_size; // 16 inode per block

private:
	static std::shared_ptr<DiskManager> instance;

	bool m_is_on;

	std::string m_disk_path;

	SuperBlock m_super_block;
	FAT m_fat;
	AllocMap<file_block_count> m_inode_map;
	AllocMap<file_block_count> m_block_map;

	std::unordered_map<int, int> m_threads;
	std::unordered_map<int, ShellInfo> m_shells;
	std::unordered_map<std::string, User> m_users;
	std::unordered_map<int, FileStatus> m_file_status;

public:
	DiskManager();

	~DiskManager();

	void start();

	int open(int fid, std::string mode, int sid);

	int close(int fid, std::string mode);

private:
	DiskBlock readBlock(int id);

	void writeBlock(int id, DiskBlock &block);

	void initDisk();

	void boot();

	void checkFiles();

	void shutdown();

	void loadSuperBlock();

	void loadFAT();

	void loadINodeMap();

	void loadBlockMap();

	void loadUsers();

	void killThreads();

	void saveSuperBlock();

	void saveFAT();

	void saveINodeMap();

	void saveBlockMap();

	void initCWD();

	std::string timeToDate(const std::chrono::system_clock::time_point &time);

	bool checkAndCreate(std::string path, FileType type);

	std::shared_ptr<IndexNode> getIndexNode(int id);

	void writeIndexNode(int id, std::shared_ptr<IndexNode> inode);

	bool openDirectory(std::shared_ptr<IndexNode> inode, int sid);

	std::shared_ptr<DirectoryEntry> getDirectoryEntry(std::string path, int sid);

	std::shared_ptr<File> getFile(std::shared_ptr<IndexNode> inode);

	void writeFile(std::shared_ptr<IndexNode> inode, std::shared_ptr<File> file);

	std::string readFile(std::string path);

	void updateTime(std::shared_ptr<IndexNode> inode, char type);

	int getDirSize(std::shared_ptr<IndexNode> inode);

	int expandFileSize(std::shared_ptr<IndexNode> inode, int size);

	int getLastBlock(int id);

	int allocIndexNode();

	int allocFileBlock(int n);

	int expandBlock(int id, int n);

	void freeIndexNode(int id);

	void freeFlieBlock(int id);

	int exec(std::string command, int sid);

	bool checkPermission(Permission need, std::shared_ptr<IndexNode> inode, int uid);

	std::function<int(int, char **, int)> getFunc(std::string command_name);

	std::vector<std::string> splitArgs(std::string command);

	std::vector<std::string> splitPath(std::string path);

	std::string getPath(std::string cur, std::string path);

	void listenLogin();

	void accept(Requset req);

	void run(int sid);

	void writeOutput(std::string out, int sid);

	std::string fill(std::string s, int w, char f);

	// commands
	int info(int argc, char *argv[], int sid);
	int cd(int argc, char *argv[], int sid);
	int dir(int argc, char *argv[], int sid);
	int md(int argc, char *argv[], int sid);
	int rd(int argc, char *argv[], int sid, int inode_id, int block_id);
	int newfile(int argc, char *argv[], int sid);
	int cat(int argc, char *argv[], int sid);
	int copy(int argc, char *argv[], int sid);
	int del(int argc, char *argv[], int sid, int inode_id, int block_id);
	int check(int argc, char *argv[], int sid);
	int write(int argc, char *argv[], int sid);
	int chmod(int argc, char *argv[], int sid);
};

#endif // __DISK_MANAGER_H