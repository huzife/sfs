#include "simdisk/disk-manager.h"

static const struct option long_options[] = {
	{"append", no_argument, nullptr, 'a'},
	{"newline", no_argument, nullptr, 'n'},
	{"delay", no_argument, nullptr, 'd'},
	{nullptr, no_argument, nullptr, 0}};

int DiskManager::write(int argc, char *argv[], int sid) {
	optind = 0;
	// get options
	int ch;
	bool append = false;
	bool newline = false;
	bool delay = false;
	while ((ch = getopt_long(argc, argv, "and", long_options, nullptr)) != -1) {
		switch (ch) {
		case 'a':
			append = true;
			break;
		case 'n':
			newline = true;
			break;
		case 'd':
			delay = true;
			break;
		default:
			std::string out = "invalid option '" + std::string(argv[optind - 1]) + "'\n";
			if (sid != 0)
				writeOutput(out, sid);
			return -1;
		}
	}

	if (optind == argc) {
		writeOutput("write: missing operand", sid);
		return -1;
	}

	if (optind == argc - 1) {
		writeOutput("write: missing content", sid);
		return -1;
	}

	std::string path, content;
	path = argv[optind];
	content = argv[optind + 1];

	if (newline) content += '\n';

	auto dentry = getDirectoryEntry(path, sid);
	if (dentry == nullptr) return -1;

	open(dentry->m_inode, "w", sid);
	if (delay) {
		std::this_thread::sleep_for(std::chrono::seconds(3));
	}

	auto inode = getIndexNode(dentry->m_inode);
	if (inode->m_type == FileType::DIRECTORY || inode->m_type == FileType::LINK) {
		writeOutput("write: " + dentry->m_filename + ": Is a directory", sid);
		close(dentry->m_inode, "w");
		return -1;
	}

	if (!checkPermission(Permission::WRITE, inode, m_shells[sid].m_user)) {
		writeOutput("write: cannot write file '" + dentry->m_filename + "': Permission denied", sid);
		close(dentry->m_inode, "w");
		return -1;
	}

	int old_size = inode->m_size;
	int inc_size = append ? content.size() : content.size() - old_size;
	if (inc_size > 0) {
		int ret = expandFileSize(inode, inc_size);
		if (ret == -1) {
			writeOutput("error: no enough blocks", sid);
			close(dentry->m_inode, "w");
			return -1;
		}
	}
	else {
		inode->m_size = content.size();
	}

	int size = inode->m_blocks * DiskManager::block_size;
	auto file = std::dynamic_pointer_cast<DataFile>(getFile(inode));
	std::shared_ptr<char[]> buffer;
	if (append) {
		buffer = file->dump();
		memcpy(buffer.get() + old_size, content.data(), content.size());
	}
	else {
		buffer.reset(new char[size]);
		memcpy(buffer.get(), content.data(), content.size());
	}
	file->load(buffer);
	writeFile(inode, file);
	writeIndexNode(dentry->m_inode, inode);
	writeOutput("write: success", sid);

	close(dentry->m_inode, "w");

	return 0;
}