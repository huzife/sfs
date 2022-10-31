#include "simdisk/initializor.h"
#include "simdisk/disk-manager.h"

void Initializor::init() {
	if (this->exist())
		std::cout << "file system already exists" << std::endl;
	else {
		std::cout << "file system does not exist, creating..." << std::endl;
		create();
	}
}

bool Initializor::exist() {
	std::ifstream ifs(m_path);
	return ifs.is_open();
}

void Initializor::create() {
	std::ofstream ofs(m_path, std::ios::out | std::ios::binary);

	// create an empty file and write 100 MiB
	// warn: must use loop instead of a 100 MiB char array!!!
	if (!ofs.is_open())
		std::cerr << "could not open disk file" << std::endl;

	char buffer[m_block_size];
	for (int i = 0; i < m_block_count; i++) {
		ofs.write(buffer, sizeof(buffer));
	}
	ofs.close();
	format();
}

void Initializor::format() {
	initSuperBlock();
	initFAT();
	initAllocMap();
	initRoot();
}

void Initializor::initSuperBlock() {
	SuperBlock s;
	s.m_fat_location = 1;
	s.m_fat_size = DiskManager::fat_size / DiskManager::block_size;
	s.m_block_map_location = 401;
	s.m_block_map_size = 12;
	s.m_inode_map_location = 413;
	s.m_inode_map_size = 12;
	s.m_block_size = DiskManager::block_size;
	s.m_filename_maxbytes = 255;
	s.m_total_block = DiskManager::file_block_count;
	s.m_total_inode = DiskManager::file_block_count;
	s.m_free_block = DiskManager::file_block_count - 1;
	s.m_free_inode = DiskManager::inode_block_count - 1;

	s.m_root.m_filename = "/";
	s.m_root.m_inode = 0;
	s.m_root.m_name_len = s.m_root.m_filename.size();
	s.m_root.m_rec_len = 7 + s.m_root.m_name_len;

	auto buffer = s.dump();
	DiskBlock block(0);
	block.setData(buffer, 0);
	DiskManager::getInstance()->writeBlock(0, block);
}

void Initializor::initFAT() {
	FAT t(DiskManager::block_count, -1);
	for (int i = 0; i < DiskManager::fat_block_count - 1; i++) {
		t[i + 1] = i + 2;
	}
	for (int i = 0; i < 11; i++) {
		t[i + 401] = i + 402;
		t[i + 413] = i + 414;
	}

	auto buffer = t.dump();
	for (int i = 0; i < DiskManager::fat_block_count; i++) {
		DiskBlock block(i + 1);
		block.setData(buffer, i * DiskManager::block_size);
		DiskManager::getInstance()->writeBlock(i + 1, block);
	}
}

void Initializor::initAllocMap() {
	// size of allocation map of block and inode are same
	AllocMap<DiskManager::file_block_count> m;
	m.reset();
	m.set(0);
	auto buffer = m.dump();
	for (int i = 0; i < 12; i++) {
		DiskBlock block1(i + 401);
		DiskBlock block2(i + 413);
		block1.setData(buffer, i * DiskManager::block_size);
		block2.setData(buffer, i * DiskManager::block_size);
		DiskManager::getInstance()->writeBlock(i + 401, block1);
		DiskManager::getInstance()->writeBlock(i + 413, block2);
		if (i == 0) {
			buffer[0] = 0;
		}
	}
}

void Initializor::initRoot() {
	// init inode of root path
	IndexNode n;
	n.m_type = FileType::DIRECTORY;                    // "/" path is a directory
	n.m_owner_permission = static_cast<Permission>(7); // owner permission: rwx
	n.m_other_permission = static_cast<Permission>(5); // other permission: r-x
	n.m_owner = 0;                                     // owner: 0(root)
	n.m_size = DiskManager::block_size;                // directory takes up one block at begin
	n.m_location = 0;                                  // first file block
	n.m_count = 1;                                     // 1 hard link
	n.m_subs = 2;                                      // "." and ".."

	auto now = std::chrono::system_clock::now(); // set up times to now
	n.m_create_time = now;
	n.m_access_time = now;
	n.m_modify_time = now;
	n.m_change_time = now;
	DiskBlock inode_block(DiskManager::inode_block_offset);
	inode_block.setData(n.dump(), 0, DiskManager::inode_size);
	DiskManager::getInstance()->writeBlock(DiskManager::inode_block_offset, inode_block);

	// init dir file of root path
	DirFile d(DiskManager::block_size);
	DirectoryEntry temp;
	temp.m_filename = "/";
	temp.m_inode = 0;
	temp.m_name_len = temp.m_filename.size();
	temp.m_rec_len = 7 + temp.m_name_len;
	d.m_parent = temp;
	d.m_current = temp;

	DiskBlock file_block(DiskManager::file_block_offset);
	file_block.setData(d.dump(), 0);
	DiskManager::getInstance()->writeBlock(DiskManager::file_block_offset, file_block);
}