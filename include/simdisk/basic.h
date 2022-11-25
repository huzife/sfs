#ifndef __BASIC_H
#define __BASIC_H

#include <memory>
#include <bitset>
#include <string>
#include <vector>
#include <map>
#include <atomic>
#include <cstdlib>
#include <chrono>
#include <string.h>
#include <assert.h>
#include <iostream> // for debug
#include <unistd.h>
#include <sys/types.h>
#include <semaphore.h>
#include "simdisk/communication.h"

class DiskManager;

// define a disk block
class DiskBlock {
public:
	static constexpr int byte_size = 1024;		   // 1 KiB per block
	static constexpr int bit_size = byte_size * 8; // 8 Kib per block
private:
	int m_id;
	std::bitset<bit_size> m_data;

public:
	DiskBlock() : m_id(-1), m_data(std::bitset<bit_size>()) {}

	DiskBlock(int id) : m_id(id), m_data(std::bitset<bit_size>()) {}

	int getID();

	std::bitset<bit_size> getBitData();

	void getData(std::shared_ptr<char[]> data, int offset);

	void getData(std::shared_ptr<char[]> data, int offset, int size);

	void setData(std::shared_ptr<char[]> data, int offset);

	void setData(std::shared_ptr<char[]> data, int offset, int size);
};

// define a Data type to implement binary data saving and loading
class Data {
public:
	virtual std::shared_ptr<char[]> dump() = 0;

	virtual void load(std::shared_ptr<char[]> buffer) = 0;
};

// define file allocation table
class FAT : public Data {
private:
	std::vector<int> m_table;

public:
	FAT(size_t size) : m_table(size) {}

	FAT(size_t size, int val) : m_table(size, val) {}

	int &operator[](size_t n) { return m_table[n]; }

	std::shared_ptr<char[]> dump() override;

	void load(std::shared_ptr<char[]> buffer) override;
};

// define block allocation map
template <size_t N>
class AllocMap : public Data {
public:
	std::bitset<N> m_map;

	// overload some function provide by bitset
	void set() { m_map.set(); }
	void set(size_t pos, bool val = true) { m_map.set(N - pos - 1, val); }
	void reset() { m_map.reset(); }
	void reset(size_t pos) { m_map.reset(N - pos - 1); }
	bool test(size_t pos) { return m_map.test(N - pos - 1); }
	size_t size() { return m_map.size(); }
	std::bitset<N>::reference operator[](size_t pos) { return m_map[N - pos - 1]; }

	// get the size that this map takes up in disk, which is a multiple of size of a block
	int getSize() {
		int size = sizeof(m_map);
		if (size % DiskBlock::byte_size == 0)
			return size;
		return (size / DiskBlock::byte_size + 1) * DiskBlock::byte_size;
	}

	std::shared_ptr<char[]> dump() override {
		int s = getSize();
		std::shared_ptr<char[]> ret(new char[s]);
		memcpy(ret.get(), &m_map, s);
		return ret;
	}

	void load(std::shared_ptr<char[]> buffer) override {
		memcpy(&m_map, buffer.get(), sizeof(m_map));
	}
};

// there are 7 file types in Linux
enum class FileType : int8_t {
	NORMAL,	   // normal file
	DIRECTORY, // directory
	BLOCK,	   // block device
	CHARACTER, // character device
	LINK,	   // symbolic link
	PIPE,	   // pipe file
	SOCKET	   // socket file
};

enum class Permission : int8_t {
	EXEC = 1,
	WRITE = 2,
	READ = 4
};

// define index node
class IndexNode : public Data {
public:
	FileType m_type;									 // file type
	Permission m_owner_permission;						 // owner permission
	Permission m_other_permission;						 // other permission
	uint16_t m_owner;									 // owner
	int m_size;											 // file size(Byte)
	int m_subs;											 // number of sub directories
	int m_blocks;										 // number of block the file takes
	int m_location;										 // file location on disk
	int m_count;										 // hard link count
	std::chrono::system_clock::time_point m_create_time; // file create time
	std::chrono::system_clock::time_point m_access_time; // last access time
	std::chrono::system_clock::time_point m_modify_time; // last modify time
	std::chrono::system_clock::time_point m_change_time; // last change time

	IndexNode() = default;

	IndexNode(const IndexNode &) = default;

	IndexNode &operator=(const IndexNode &) = default;

	IndexNode(FileType type, uint16_t owner, int loc) : m_type(type), m_owner(owner), m_location(loc) {
		m_owner_permission = static_cast<Permission>(7);
		m_other_permission = static_cast<Permission>(5);
		m_size = type == FileType::DIRECTORY ? 1024 : 0;
		m_subs = type == FileType::DIRECTORY ? 2 : 1;
		m_blocks = 1;
		m_count = 1;

		auto now = std::chrono::system_clock::now();
		m_create_time = now;
		m_access_time = now;
		m_modify_time = now;
		m_change_time = now;
	}

	IndexNode(FileType type, uint16_t owner, int loc, int size, int blocks)
		: IndexNode(type, owner, loc) {
		m_size = size;
		m_blocks = blocks;
	}

	static std::string FileTypeToStr(FileType type);

	static std::string PermissionToStr(Permission p);

	static bool PermissionCheck(Permission p, Permission r);

	std::shared_ptr<char[]> dump() override;

	void load(std::shared_ptr<char[]> buffer) override;
};

// define directory entry(fcb)
class DirectoryEntry : Data {
public:
	int m_inode;
	uint16_t m_rec_len;
	uint8_t m_name_len;
	std::string m_filename;

	DirectoryEntry() = default;

	DirectoryEntry(const DirectoryEntry &) = default;

	DirectoryEntry(std::string filename)
		: m_filename(filename), m_name_len(filename.size()), m_rec_len(7 + m_name_len) {}

	std::shared_ptr<char[]> dump() override;

	void load(std::shared_ptr<char[]> buffer) override;
};

// define super block
class SuperBlock : public Data {
public:
	int m_fat_location;		  // FAT location on disk
	int m_fat_size;			  // FAT size(blocks)
	int m_block_map_location; // block map location on disk
	int m_block_map_size;	  // block map size(blocks)
	int m_inode_map_location; // inode map location on disk
	int m_inode_map_size;	  // inode map size(blocks)

	int m_block_size;		 // size per block(Byte)
	int m_filename_maxbytes; // max length of filename(Byte)
	int m_total_block;		 // total count of block
	int m_total_inode;		 // total count of index node
	int m_free_block;		 // free blocks left
	int m_free_inode;		 // free inodes left

	DirectoryEntry m_root; // dEntry of root path(/)

	bool m_dirt; // record if the super block has been modified

	std::shared_ptr<char[]> dump() override;

	void load(std::shared_ptr<char[]> buffer) override;
};

// define file
class File : public Data {
public:
	int m_size;
	int m_subs;

	File(int size, int subs) : m_size(size), m_subs(subs) {}
};

class DataFile : public File {
public:
	std::shared_ptr<char[]> m_data;

	DataFile(int size) : File(size, 1), m_data(new char[size]) {}

	std::shared_ptr<char[]> dump() override;

	void load(std::shared_ptr<char[]> buffer) override;
};

class DirFile : public File {
public:
	DirectoryEntry m_parent;
	DirectoryEntry m_current;
	std::map<std::string, DirectoryEntry> m_dirs;

	DirFile(int size, int subs = 2) : File(size, subs), m_parent(".."), m_current(".") {}

	std::shared_ptr<char[]> dump() override;

	void load(std::shared_ptr<char[]> buffer) override;
};

class FileStatus {
public:
	std::atomic<int> count;
	sem_t rw;
	sem_t w;

	FileStatus() : count(0) {
		sem_init(&rw, 0, 1);
		sem_init(&w, 0, 1);
	}

	void write();

	void finishWrite();

	void read();

	void finishRead();
};

// current working directories
class ShellInfo {
public:
	int m_user;
	ShareMemory *m_shm;
	std::string m_path;
	std::shared_ptr<DirectoryEntry> m_dentry;
	std::shared_ptr<IndexNode> m_inode;

	ShellInfo(int user = 0) : m_user(user) {}

	ShellInfo &operator=(const ShellInfo &s) {
		m_user = s.m_user;
		m_shm = s.m_shm;
		m_path = s.m_path;
		m_dentry = s.m_dentry;
		m_inode = s.m_inode;
		return *this;
	}
};

// record users info
class User {
public:
	std::string m_name;	  // user name
	std::string m_passwd; // password
	int m_uid;			  // user id
	std::string m_home;	  // user home directory
};

#if 0
// hash function of User
class UserHash{
public:
	size_t operator()(const User &user) const {
		return std::hash<std::string>()(user.m_name);
	}
};
#endif

#endif // __BASIC_H