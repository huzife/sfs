#include "simdisk/basic.h"
#include "simdisk/disk-manager.h" // for some constant

int DiskBlock::getID() {
	return m_id;
}

std::bitset<DiskBlock::bit_size> DiskBlock::getBitData() {
	return m_data;
}

void DiskBlock::getData(std::shared_ptr<char[]> data, int offset) {
	for (int i = 0; i < byte_size; i++) {
		char ch = 0;
		for (int j = 0; j < 8; j++) {
			if (m_data.test(i * 8 + j))
				ch |= (1 << (7 - j));
		}
		data[i + offset] = ch;
	}
}

void DiskBlock::getData(std::shared_ptr<char[]> data, int offset, int size) {
	assert(offset + size <= byte_size);
	for (int i = 0; i < size; i++) {
		char ch = 0;
		for (int j = 0; j < 8; j++) {
			if (m_data.test((i + offset) * 8 + j))
				ch |= (1 << (7 - j));
		}
		data[i] = ch;
	}
}

void DiskBlock::setData(std::shared_ptr<char[]> data, int offset) {
	for (int i = 0; i < byte_size; i++) {
		char ch = data[i + offset];
		for (int j = 0; j < 8; j++) {
			m_data.set(i * 8 + j, ch & (1 << (7 - j)));
		}
	}
}

void DiskBlock::setData(std::shared_ptr<char[]> data, int offset, int size) {
	assert(offset + size <= byte_size);
	for (int i = 0; i < size; i++) {
		char ch = data[i];
		for (int j = 0; j < 8; j++) {
			m_data.set((i + offset) * 8 + j, ch & (1 << (7 - j)));
		}
	}
}

// FAT
std::shared_ptr<char[]> FAT::dump() {
	std::shared_ptr<char[]> ret(new char[m_table.size() * 4]);
	memcpy(ret.get(), m_table.data(), m_table.size() * 4);

	return ret;
}

void FAT::load(std::shared_ptr<char[]> buffer) {
	m_table.resize(DiskManager::block_count);
	memcpy(m_table.data(), buffer.get(), DiskManager::fat_size);
}

// IndexNode
std::string IndexNode::FileTypeToStr(FileType type) {
	switch (type) {
	case FileType::NORMAL:
		return "-";
	case FileType::DIRECTORY:
		return "d";
	case FileType::BLOCK:
		return "b";
	case FileType::CHARACTER:
		return "c";
	case FileType::LINK:
		return "l";
	case FileType::PIPE:
		return "p";
	case FileType::SOCKET:
		return "s";
	}
	return "";	// unreachable, but received a warning without it
}

std::string IndexNode::PermissionToStr(Permission p) {
	std::string ret = "---";
	if (PermissionCheck(p, Permission::READ))
		ret[0] = 'r';
	if (PermissionCheck(p, Permission::WRITE))
		ret[1] = 'w';
	if (PermissionCheck(p, Permission::EXEC))
		ret[2] = 'x';

	return ret;
}

bool IndexNode::PermissionCheck(Permission p, Permission r) {
	return static_cast<int8_t>(p) & static_cast<int8_t>(r);
}

std::shared_ptr<char[]> IndexNode::dump() {
	std::shared_ptr<char[]> ret(new char[DiskManager::inode_size]);
	memcpy(ret.get(), reinterpret_cast<char *>(this) + sizeof(Data), sizeof(IndexNode) - sizeof(Data));

	return ret;
}

void IndexNode::load(std::shared_ptr<char[]> buffer) {
	memcpy(reinterpret_cast<char *>(this) + sizeof(Data), buffer.get(), sizeof(IndexNode) - sizeof(Data));
}

// DirectoryEntry
std::shared_ptr<char[]> DirectoryEntry::dump() {
	std::shared_ptr<char[]> ret(new char[DiskManager::block_size]);
	memcpy(ret.get(), reinterpret_cast<char *>(this) + sizeof(Data), 7);
	memcpy(ret.get() + 7, m_filename.data(), m_name_len);

	return ret;
}

void DirectoryEntry::load(std::shared_ptr<char[]> buffer) {
	memcpy(reinterpret_cast<char *>(this) + sizeof(Data), buffer.get(), 7);
	m_filename.assign(buffer.get() + 7, m_name_len);
}

// SuperBlock
std::shared_ptr<char[]> SuperBlock::dump() {
	std::shared_ptr<char[]> ret(new char[DiskManager::block_size]);
	memcpy(ret.get(), reinterpret_cast<char *>(this) + sizeof(Data), 48);
	memcpy(ret.get() + 48, m_root.dump().get(), m_root.m_rec_len);

	return ret;
}

void SuperBlock::load(std::shared_ptr<char[]> buffer) {
	memcpy(reinterpret_cast<char *>(this) + sizeof(Data), buffer.get(), 48);
	uint16_t root_size = *(uint16_t *)(buffer.get() + 52);
	std::shared_ptr<char[]> temp(new char[DiskManager::block_size]);
	memcpy(temp.get(), buffer.get() + 48, root_size);
	m_root.load(temp);
	m_dirt = false;
}

// DataFile
std::shared_ptr<char[]> DataFile::dump() {
	return m_data;
}

void DataFile::load(std::shared_ptr<char[]> buffer) {
	m_data = buffer;
}

// DirFile
std::shared_ptr<char[]> DirFile::dump() {
	std::shared_ptr<char[]> ret(new char[m_size]);
	int offset = 0;

	memcpy(ret.get(), m_parent.dump().get(), m_parent.m_rec_len);
	offset += m_parent.m_rec_len;
	memcpy(ret.get() + offset, m_current.dump().get(), m_current.m_rec_len);
	offset += m_current.m_rec_len;

	for (auto &[name, d] : m_dirs) {
		memcpy(ret.get() + offset, d.dump().get(), d.m_rec_len);
		offset += d.m_rec_len;
	}

	return ret;
}

void DirFile::load(std::shared_ptr<char[]> buffer) {
	int offset = 0;
	uint16_t len;
	std::shared_ptr<char[]> temp(new char[DiskManager::block_size]);
	// m_parent
	len = *(uint16_t *)(buffer.get() + offset + 4);
	memcpy(temp.get(), buffer.get() + offset, len);
	m_parent.load(temp);
	offset += len;
	// m_current
	len = *(uint16_t *)(buffer.get() + offset + 4);
	memcpy(temp.get(), buffer.get() + offset, len);
	m_current.load(temp);
	offset += len;
	// m_dirs
	int cnt = 2;
	while (cnt < m_subs) {
		len = *(uint16_t *)(buffer.get() + offset + 4);
		memcpy(temp.get(), buffer.get() + offset, len);
		DirectoryEntry d;
		d.load(temp);
		m_dirs[d.m_filename] = d;
		offset += len;
		cnt++;
	}
}